#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""retrait_iar.py — retrait des fichiers IAR du dépôt (étape 6, tâche 3).

Sélectionne, parmi les fichiers versionnés sous scion/ (git ls-files) :
  - projet : tout sys/root/prj/ (iar, scons, vc-2010, config ; décision 2026-10-05) et
    sys/user/tauon-basic/prj/iar/ ;
  - fichier IAR : extensions d'outillage IAR (.ewp .eww .ewd .ewt .icf .xcl .mac .s79 .dni .wsdt
    .dbgdt, .bat C-SPY) et assembleur .s/.asm en syntaxe IAR (analyse_asm d'audit_iar.py),
    hors code tiers (ORIGINE_TIERS d'audit_iar.py, third_party/ : paquets laissés intacts,
    décision 2026-10-05) et hors code gelé (supprimé par retrait_gele.py, tag legacy).
Écrit la liste dans doc/migration/retrait-iar.md (fichiers retirés, fichiers IAR conservés et
motif). --apply supprime les fichiers sélectionnés par `git rm` (sans commit) et retire leurs
lignes de doc/migration/perimetre.csv. Rejouable : un fichier déjà retiré n'est plus listé.

Usage (racine du clone) :
  python3 tools/migration/retrait_iar.py            # simulation, écrit la liste
  python3 tools/migration/retrait_iar.py --apply    # git rm des fichiers sélectionnés
"""
import argparse
import collections
import csv
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import audit_iar  # noqa: E402

CLONE_ROOT = audit_iar.CLONE_ROOT
SORTIE = os.path.join(CLONE_ROOT, "doc", "migration", "retrait-iar.md")
PROJETS = ("sys/root/prj/", "sys/user/tauon-basic/prj/iar/")
EXT_IAR = {".ewp", ".eww", ".ewd", ".ewt", ".icf", ".xcl", ".mac", ".s79", ".dni", ".wsdt",
           ".dbgdt"}
EXT_ASM = {".s", ".asm"}


def fichiers_scion():
    sortie = subprocess.run(["git", "-C", CLONE_ROOT, "ls-files", "-z", "scion/"],
                            capture_output=True, text=True, check=True).stdout
    return [f[len("scion/"):] for f in sortie.split("\0") if f]


def nature_iar(rel):
    """Motif IAR du fichier, ou None."""
    if rel.startswith(PROJETS):
        return "projet"
    nom = os.path.basename(rel).lower()
    ext = os.path.splitext(nom)[1]
    if ext in EXT_IAR:
        return "outillage IAR (%s)" % ext
    if ext == ".bat" and "cspy" in nom:
        return "outillage IAR (C-SPY .bat)"
    if ext in EXT_ASM:
        texte = audit_iar.lire(os.path.join(CLONE_ROOT, "scion", rel))
        if texte is not None and audit_iar.analyse_asm(rel, texte)[1] == "IAR":
            return "assembleur IAR"
    return None


def selectionner(vent):
    retirer, garder = [], []
    for rel in fichiers_scion():
        nature = nature_iar(rel)
        if not nature:
            continue
        ens = vent.classer(rel)[0]
        if nature != "projet" and ens == "gele":
            garder.append((rel, nature, "code gelé : retrait_gele.py (tag legacy)"))
        elif nature != "projet" and (rel.startswith("third_party/")
                                     or audit_iar.ORIGINE_TIERS.search(rel)):
            garder.append((rel, nature, "paquet tiers laissé intact (décision 2026-10-05)"))
        else:
            retirer.append((rel, nature, ens))
    return retirer, garder


def elaguer_perimetre():
    """Retire de perimetre.csv les lignes des fichiers qui ne sont plus versionnés (lignes
    restantes inchangées octet à octet). Retourne le nombre de lignes retirées."""
    presents = set(fichiers_scion())
    with open(audit_iar.DEFAULT_PERIMETRE, encoding="utf-8", newline="") as f:
        lignes = f.readlines()
    garder = lignes[:1]
    for ligne in lignes[1:]:
        p = next(csv.reader([ligne]))[0]
        if p.split("/", 1)[0] in ("src", "prj"):
            p = "sys/root/" + p
        if p in presents or p.endswith("/"):
            garder.append(ligne)
    with open(audit_iar.DEFAULT_PERIMETRE, "w", encoding="utf-8", newline="") as f:
        f.writelines(garder)
    return len(lignes) - len(garder)


def ecrire_liste(retirer, garder):
    L = ["# Retrait IAR — étape 6", "",
         "Généré par `tools/migration/retrait_iar.py` (rejouable). État précédent : tag local "
         "`legacy-iar`. Chemins relatifs au trunk.", "",
         "Fichiers retirés : **%d** ; fichiers IAR conservés : **%d**." % (len(retirer),
                                                                          len(garder)), "",
         "## Retirés, par répertoire", "", "| Répertoire | Motif | Fichiers |", "|---|---|---|"]
    par_rep = collections.Counter()
    for rel, nature, _ in retirer:
        rep = "/".join(rel.split("/")[:4 if rel.startswith("sys/root/prj/") else 5])
        par_rep[(rep, nature.split(" (")[0])] += 1
    for (rep, nature), n in sorted(par_rep.items()):
        L.append("| `%s` | %s | %d |" % (rep, nature, n))
    L += ["", "## IAR conservés (hors retrait)", "", "| Motif | Fichiers | Exemple |", "|---|---|---|"]
    par_motif = collections.defaultdict(list)
    for rel, nature, motif in garder:
        par_motif[motif].append(rel)
    for motif, liste in sorted(par_motif.items()):
        L.append("| %s | %d | `%s` |" % (motif, len(liste), sorted(liste)[0]))
    L += ["", "## Liste complète des fichiers retirés", ""]
    L += ["- `%s` (%s, %s)" % r for r in sorted(retirer)]
    with open(SORTIE, "w", encoding="utf-8") as f:
        f.write("\n".join(L) + "\n")


def main():
    ap = argparse.ArgumentParser(description="Retrait des fichiers IAR (étape 6).")
    ap.add_argument("--apply", action="store_true", help="git rm des fichiers sélectionnés")
    a = ap.parse_args()
    vent = audit_iar.Ventilateur("auto", audit_iar.DEFAULT_PERIMETRE)
    retirer, garder = selectionner(vent)
    if retirer or not os.path.exists(SORTIE):
        ecrire_liste(retirer, garder)
    print("retrait_iar : %d fichier(s) à retirer, %d fichier(s) IAR conservé(s) ; liste : %s"
          % (len(retirer), len(garder), os.path.relpath(SORTIE, CLONE_ROOT)))
    for nature, n in sorted(collections.Counter(r[1].split(" (")[0] for r in retirer).items()):
        print("  %-20s %d" % (nature, n))
    if a.apply and retirer:
        chemins = ["scion/" + r[0] for r in retirer]
        for i in range(0, len(chemins), 200):
            subprocess.run(["git", "-C", CLONE_ROOT, "rm", "-q", "--"] + chemins[i:i + 200],
                           check=True)
        print("retrait_iar : %d fichier(s) retiré(s) (git rm, sans commit)" % len(chemins))
    if a.apply:
        print("retrait_iar : perimetre.csv : %d ligne(s) de fichiers retirés supprimée(s)"
              % elaguer_perimetre())


if __name__ == "__main__":
    main()
