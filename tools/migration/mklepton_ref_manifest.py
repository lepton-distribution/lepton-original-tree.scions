#!/usr/bin/env python3
"""mklepton_ref_manifest.py — jeu de sorties de référence mklepton, voie de repli (étape 1, tâche 5).

Décision utilisateur du 2026-09-30 : l'oracle binaire (mklepton_gnu) n'est pas exécutable
(libkernel.so i386 absente) ; la référence est constituée des sorties mklepton déjà versionnées
dans l'arbre. Ce script les recense (chemin, taille, sha256, blob git, mkconf d'origine) et écrit
doc/migration/mklepton-ref.md. Il n'écrit rien dans le trunk ; les fichiers restent dans l'arbre
(et dans l'historique git du clone), ils ne sont pas copiés.

Usage (racine du clone) : python3 tools/migration/mklepton_ref_manifest.py
"""
import hashlib
import os
import re
import subprocess

TRUNK = os.path.realpath(os.path.expanduser(os.environ.get("LEPTON_TRUNK", "~/lepton/trunk")))
CLONE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OUT = os.path.join(CLONE, "doc", "migration", "mklepton-ref.md")

# Sorties de mklepton (mklepton.md §4) ; les user_kernel_mkconf.h sont des entrées écrites à la main.
OUTPUT_NAMES = re.compile(r"^(kernel_mkconf\.h|dev_mkconf\.c|bin_mkconf\.c|dev_dskimg\.[ch]|\.boot|\.mount)$")
ORIGIN = re.compile(r"generated from (\S+) by mklepton")


def git_blob(path):
    rel = os.path.relpath(os.path.realpath(path), os.path.join(CLONE, "scion"))
    try:
        return subprocess.check_output(["git", "-C", CLONE, "rev-parse", "HEAD:scion/" + rel],
                                       text=True, stderr=subprocess.DEVNULL).strip()[:12]
    except subprocess.CalledProcessError:
        return "non versionné"


def main():
    rows = []
    for root, dirs, files in os.walk(TRUNK, followlinks=True):
        dirs.sort()
        for name in sorted(files):
            if not OUTPUT_NAMES.match(name):
                continue
            path = os.path.join(root, name)
            data = open(path, "rb").read()
            m = ORIGIN.search(data[:4096].decode("latin-1"))
            rows.append((os.path.relpath(path, TRUNK), len(data), hashlib.sha256(data).hexdigest(),
                         git_blob(path), m.group(1) if m else "—"))
    L = ["# Sorties de référence mklepton — voie de repli",
         "",
         "Généré par `tools/migration/mklepton_ref_manifest.py`. Décision utilisateur du 2026-09-30 : "
         "l'oracle binaire `mklepton_gnu` n'est pas exécutable (`libkernel.so` i386 absente, non "
         "reconstructible) ; la copie historique hors arbre n'est pas utilisée. La référence est le "
         "jeu de sorties déjà versionnées ci-dessous (non copiées : blob git au commit courant).",
         "",
         "| Fichier (relatif au trunk) | Octets | sha256 | Blob git | mkconf d'origine |",
         "|---|---|---|---|---|"]
    for r in rows:
        L.append("| `%s` | %d | `%s…` | `%s` | `%s` |" % (r[0], r[1], r[2][:16], r[3], r[4]))
    L += ["",
          "## Usage à l'étape 2",
          "",
          "- Format du C généré (`kernel_mkconf.h`, `dev_mkconf.c`, `bin_mkconf.c`, `dev_dskimg.[ch]`) : "
          "comparaison structurelle avec la sortie du mklepton natif (mêmes macros, tables `dev_lst`, "
          "`_bin_lst`, tableau `filecpu_memory[]`), chemins et horodatage exclus.",
          "- `dev_dskimg.c` (win32) : image UFS au format MSVC `pack(1)` (nœud de 18 o contre 24 o sous "
          "GCC, `noyau-statique.md`) ; utilisable pour décoder la structure (superbloc `ufs 1.5`), pas "
          "pour une comparaison octet à octet. Son mkconf d'origine (`tauon-nuodio`) est absent de l'arbre.",
          "- `.boot` / `.mount` : texte, comparaison directe.",
          "- Image UFS produite par le mklepton natif : validée par exécution (montage et lecture sous "
          "QEMU, étape 3), pas par comparaison binaire.",
          ""]
    with open(OUT, "w", encoding="utf-8") as fh:
        fh.write("\n".join(L))
    print("écrit : %s (%d fichiers)" % (os.path.relpath(OUT, CLONE), len(rows)))


if __name__ == "__main__":
    main()
