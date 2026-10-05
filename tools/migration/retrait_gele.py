#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""retrait_gele.py — suppression du code gelé (étape 6, tâche 4 ; décision 2026-10-05).

Sélectionne les fichiers de l'ensemble « gelé » de doc/migration/perimetre.csv (inventaire de
code-gele.md, copies d'origine legacy/ de l'étape 4) encore versionnés sous scion/. --apply les
supprime par `git rm` (sans commit) et retire leurs lignes de perimetre.csv. État précédent :
tag local `legacy`. Rejouable : un fichier déjà retiré n'est plus sélectionné.

Usage (racine du clone) :
  python3 tools/migration/retrait_gele.py            # simulation : totaux par répertoire
  python3 tools/migration/retrait_gele.py --apply    # git rm des fichiers gelés
"""
import argparse
import collections
import csv
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import audit_iar  # noqa: E402
import retrait_iar  # noqa: E402

CLONE_ROOT = audit_iar.CLONE_ROOT


def geles():
    """Chemins (relatifs à scion/) de l'ensemble gelé de perimetre.csv."""
    out = []
    with open(audit_iar.DEFAULT_PERIMETRE, encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            if audit_iar.norm_ensemble(row["ensemble"]) != "gele":
                continue
            p = row["fichier"].strip()
            if p.split("/", 1)[0] in ("src", "prj"):
                p = "sys/root/" + p
            out.append(p)
    return out


def main():
    ap = argparse.ArgumentParser(description="Suppression du code gelé (étape 6).")
    ap.add_argument("--apply", action="store_true", help="git rm des fichiers gelés")
    a = ap.parse_args()
    presents = set(retrait_iar.fichiers_scion())
    inventaire = geles()
    retirer = sorted(p for p in inventaire if p in presents)
    print("retrait_gele : %d fichier(s) gelé(s) dans perimetre.csv, %d encore versionné(s)"
          % (len(inventaire), len(retirer)))
    par_rep = collections.Counter("/".join(p.split("/")[:5]) for p in retirer)
    for rep, n in sorted(par_rep.items()):
        print("  %-70s %d" % (rep, n))
    if a.apply and retirer:
        chemins = ["scion/" + p for p in retirer]
        for i in range(0, len(chemins), 200):
            subprocess.run(["git", "-C", CLONE_ROOT, "rm", "-q", "--"] + chemins[i:i + 200],
                           check=True)
        print("retrait_gele : %d fichier(s) retiré(s) (git rm, sans commit)" % len(chemins))
    if a.apply:
        print("retrait_gele : perimetre.csv : %d ligne(s) de fichiers retirés supprimée(s)"
              % retrait_iar.elaguer_perimetre())


if __name__ == "__main__":
    main()
