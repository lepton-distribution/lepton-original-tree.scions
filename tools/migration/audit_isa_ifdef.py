#!/usr/bin/env python3
"""audit_isa_ifdef.py — conditions de préprocesseur d'ISA, de cœur ou de puce hors des
répertoires d'architecture (ETAPE-4 tâche 3, critère « aucun #ifdef d'ISA ou de cœur hors de
kal/arch/ et des répertoires d'architecture » ; CLAUDE.md §2).

Parcourt les .c/.h du périmètre actif (`doc/migration/perimetre.csv`) et relève chaque directive
#if/#ifdef/#ifndef/#elif dont la condition mentionne une macro d'un des axes :
  isa    familles de processeur : CPU_ARM7, CPU_ARM9, CPU_CORTEXM, CPU_GNU32, CPU_WIN32, CPU_M16C62,
         __thumb__, __ARM_ARCH*, __i386__, __x86_64__ …
  coeur  __tauon_cpu_core__ et ses valeurs, __CORTEX_M, __FPU_PRESENT, __ARM_FP, __VFP_FP__,
         OS_CPU_HAS_VFP …
  puce   __tauon_cpu_device__ et ses valeurs (axe carte : autorisé dans le BSP et dev/arch)
Emplacements autorisés : kernel/core/arch/, kernel/core/kal/ (arch/ et backend/), kernel/dev/arch/,
kernel/dev/bsp/, et tout répertoire `arch/` ou `ports/` (portages, ex. lwIP). Le code tiers
(classement d'audit_iar.py) est compté à part : il n'est pas modifié (décision D1a).

Sorties : `doc/migration/isa-ifdef.md` (synthèse par fichier) et `isa-ifdef.csv` (occurrences).
"""
import argparse
import collections
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.realpath(__file__))
CLONE = os.path.realpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
from audit_iar import ORIGINE_TIERS  # noqa: E402

AXES = [
    ("coeur", re.compile(r"\b(__tauon_cpu_core\w*|__CORTEX_M\w*|__FPU_PRESENT|__FPU_USED|__ARM_FP\w*|"
                         r"__VFP_FP__|__SOFTFP__|OS_CPU_HAS_VFP)\b")),
    ("isa", re.compile(r"\b(CPU_(?:ARM7|ARM9|CORTEXM\w*|GNU32|WIN32|M16C\w*|RISCV\w*)|__thumb2?__|"
                       r"__ARM_ARCH\w*|__arm__|__i386__|__x86_64__|__riscv\w*)\b")),
    ("puce", re.compile(r"\b(__tauon_cpu_device\w*)\b")),
]
AUTORISE = re.compile(r"(^|/)(arch|ports)/|^sys/root/src/kernel/core/kal/|^sys/root/src/kernel/dev/bsp/")
DIRECTIVE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif)\b(.*)$")


def actifs(perimetre):
    for row in csv.DictReader(open(perimetre, encoding="utf-8")):
        f = row["fichier"]
        if row["ensemble"] == "actif" and f.endswith((".c", ".h")):
            yield f if f.startswith(("sys/", "tools/", "tests/", "legacy/")) else "sys/root/" + f


def lignes_logiques(text):
    buf, debut = "", None
    for no, line in enumerate(text.splitlines(), 1):
        if debut is None:
            debut = no
        if line.endswith("\\"):
            buf += line[:-1] + " "
            continue
        yield debut, buf + line
        buf, debut = "", None


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--perimetre", default=os.path.join(CLONE, "doc/migration/perimetre.csv"))
    ap.add_argument("--trunk", default=os.environ.get("LEPTON_TRUNK",
                                                      os.path.join(CLONE, "../../../../trunk")))
    ap.add_argument("--out-dir", default=os.path.join(CLONE, "doc/migration"))
    ap.add_argument("--summary", action="store_true", help="totaux seulement, aucun fichier écrit")
    a = ap.parse_args()

    occ = []
    for rel in sorted(set(actifs(a.perimetre))):
        path = os.path.join(a.trunk, rel)
        if not os.path.exists(path):
            continue
        text = open(path, encoding="latin-1").read()
        for no, line in lignes_logiques(text):
            m = DIRECTIVE.match(line)
            if not m:
                continue
            cond = re.sub(r"/\*.*?\*/|//.*", "", m.group(2)).strip()
            axes = [nom for nom, rx in AXES if rx.search(cond)]
            if not axes:
                continue
            occ.append({"fichier": rel, "ligne": no, "axes": "+".join(axes),
                        "origine": "tiers" if ORIGINE_TIERS.search(rel) else "lepton",
                        "emplacement": "autorisé" if AUTORISE.search(rel) else "hors arch",
                        "condition": " ".join(cond.split())[:160]})

    def cle(o):
        return (o["origine"], o["emplacement"])
    tot = collections.Counter(cle(o) for o in occ)
    hors = [o for o in occ if o["origine"] == "lepton" and o["emplacement"] == "hors arch"]
    # l'axe « puce » seul n'est pas un #if d'ISA ni de cœur : compté à part (axe carte)
    hors_ic = [o for o in hors if o["axes"] != "puce"]
    nf = len({o["fichier"] for o in hors_ic})
    print("audit_isa_ifdef : %d directive(s) d'ISA/cœur hors arch dans le code Lepton actif "
          "(%d fichier(s)) ; puce seule hors BSP : %d ; autorisées : %d ; tiers : %d"
          % (len(hors_ic), nf, len(hors) - len(hors_ic), tot[("lepton", "autorisé")],
             tot[("tiers", "autorisé")] + tot[("tiers", "hors arch")]))
    if a.summary:
        return 0

    os.makedirs(a.out_dir, exist_ok=True)
    with open(os.path.join(a.out_dir, "isa-ifdef.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, ["fichier", "ligne", "axes", "origine", "emplacement", "condition"])
        w.writeheader()
        w.writerows(occ)
    par_fichier = collections.defaultdict(collections.Counter)
    for o in hors:
        for ax in o["axes"].split("+"):
            par_fichier[o["fichier"]][ax] += 1
    L = ["# Conditions d'ISA, de cœur et de puce hors des répertoires d'architecture", "",
         "Généré par `tools/migration/audit_isa_ifdef.py` — ne pas éditer à la main. "
         "Occurrences : `isa-ifdef.csv`.", "",
         "Critère d'ETAPE-4 : aucune directive d'**ISA** ou de **cœur** dans le code Lepton actif "
         "hors de `kal/arch/` et des répertoires d'architecture. L'axe **puce** "
         "(`__tauon_cpu_device__`) relève de la carte : autorisé dans `dev/arch` et `dev/bsp`, "
         "listé ici pour la décomposition.", "",
         "| Mesure | Valeur |", "|---|---:|",
         "| Code Lepton, ISA/cœur hors arch (directives) | **%d** |" % len(hors_ic),
         "| Code Lepton, ISA/cœur hors arch (fichiers) | **%d** |" % nf,
         "| Code Lepton, puce seule hors arch/BSP (directives) | %d |" % (len(hors) - len(hors_ic)),
         "| Code Lepton, emplacements autorisés | %d |" % tot[("lepton", "autorisé")],
         "| Code tiers (non modifié, D1a) | %d |" % (tot[("tiers", "autorisé")] + tot[("tiers", "hors arch")]),
         "", "## Code Lepton hors arch, par fichier", "",
         "| Fichier | isa | cœur | puce |", "|---|---:|---:|---:|"]
    for f in sorted(par_fichier, key=lambda k: (-sum(par_fichier[k].values()), k)):
        c = par_fichier[f]
        L.append("| `%s` | %d | %d | %d |" % (f, c["isa"], c["coeur"], c["puce"]))
    open(os.path.join(a.out_dir, "isa-ifdef.md"), "w", encoding="utf-8").write("\n".join(L) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
