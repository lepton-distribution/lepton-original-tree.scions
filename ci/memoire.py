#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""memoire.py — occupation mémoire des exécutables d'un preset (étape 6, CI).

Lit le journal d'un `cmake --build` (ninja) dont les exécutables ont été réédités (ci/run.sh
supprime les .elf avant le build), relève les tableaux `-Wl,--print-memory-usage` de chaque
édition de liens, les ajoute au CSV d'archive et compare chaque région au seuil d'alerte.

Usage :
  ci/memoire.py --preset P --log build.log --csv memoire.csv [--seuil 90] [--attendus N]
Code de retour : 0 ; 1 si une région dépasse le seuil ou si moins de N exécutables sont relevés.
"""
import argparse
import csv
import os
import re
import sys

RE_LIEN = re.compile(r"^\[\d+/\d+\] Linking \w+ executable (\S+)")
RE_REGION = re.compile(r"^\s*(\S+):\s+(\d+)\s+(B|KB|MB|GB)\s+(\d+)\s+(B|KB|MB|GB)\s+([\d.]+)%")
UNITE = {"B": 1, "KB": 1024, "MB": 1024 ** 2, "GB": 1024 ** 3}
COLONNES = ("preset", "executable", "region", "utilise_octets", "taille_octets", "pourcent")


def relever(chemin_log):
    """Liste de (exécutable, région, utilisé, taille, pourcentage) dans l'ordre du journal."""
    lignes, courant = [], None
    with open(chemin_log, encoding="utf-8", errors="replace") as f:
        for ligne in f:
            m = RE_LIEN.match(ligne)
            if m:
                courant = os.path.basename(m.group(1))
                continue
            m = RE_REGION.match(ligne)
            if m and courant:
                lignes.append((courant, m.group(1), int(m.group(2)) * UNITE[m.group(3)],
                               int(m.group(4)) * UNITE[m.group(5)], float(m.group(6))))
    return lignes


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--preset", required=True)
    ap.add_argument("--log", required=True)
    ap.add_argument("--csv", required=True)
    ap.add_argument("--seuil", type=float, default=90.0, help="seuil d'alerte en %% par région")
    ap.add_argument("--attendus", type=int, default=1, help="nombre minimal d'exécutables")
    a = ap.parse_args()

    lignes = relever(a.log)
    nouveau = not os.path.exists(a.csv)
    with open(a.csv, "a", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        if nouveau:
            w.writerow(COLONNES)
        for exe, region, util, taille, pct in lignes:
            w.writerow((a.preset, exe, region, util, taille, "%.2f" % pct))

    echec = False
    executables = sorted({exe for exe, *_ in lignes})
    if len(executables) < a.attendus:
        print("memoire: %s : %d exécutable(s) relevé(s), %d attendu(s)"
              % (a.preset, len(executables), a.attendus), file=sys.stderr)
        echec = True
    for exe, region, util, taille, pct in lignes:
        alerte = pct > a.seuil
        echec |= alerte
        print("%-28s %-16s %-10s %10d / %10d  %6.2f %%%s"
              % (a.preset, exe, region, util, taille, pct,
                 "  ALERTE (seuil %.0f %%)" % a.seuil if alerte else ""))
    return 1 if echec else 0


if __name__ == "__main__":
    sys.exit(main())
