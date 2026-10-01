#!/usr/bin/env python3
"""axes_sbin.py — directives d'ISA de sbin (module sbin, ETAPE-4 tâche 3), règle mécanique
rejouable à contenu constant (gcc -E -P identique). Même mécanique que axes_kernel_core.py
(atomique, Latin-1 octet pour octet, analyseur de kal_split.py).

Règle :
  gelee     simulation Linux (CPU_GNU32) gelée, extension de D3a (décision KAL-2 du 2026-10-01) :
              lsh.c    fin de ligne « \\n » de la simulation Linux retirée (copie d'origine sous
                       scion/legacy/) ;
              initd.c  « (defined(EVAL_BOARD) || defined(CPU_GNU32)) && !defined(USE_KERNEL_STATIC) »
                       → « defined(EVAL_BOARD) && !defined(USE_KERNEL_STATIC) » (EVAL_BOARD est
                       une macro de carte, conservée ; condition seule : pas de copie).

Usage : axes_sbin.py <règle> [--clone <racine du clone>] [--dry-run]
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import axes_kernel_core as akc  # noqa: E402

akc.CORE = "sys/root/src/sbin"
apply, remove, replace_exact = akc.apply, akc.remove, akc.replace_exact

INITD_OLD = "#if (defined(EVAL_BOARD) || defined(CPU_GNU32)) && !defined(USE_KERNEL_STATIC)\n"
INITD_NEW = "#if defined(EVAL_BOARD) && !defined(USE_KERNEL_STATIC)\n"


def rule_gelee(clone, dry):
    def lsh(lines):
        return remove(lines, {"defined(CPU_GNU32)"}, "fin de ligne de la simulation Linux")

    def initd(lines):
        return replace_exact(lines, INITD_OLD, INITD_NEW, "console de test (CPU_GNU32 retiré)")

    print("lsh.c")
    apply(clone, "lsh.c", lsh, dry, need_legacy=True)
    print("initd.c")
    apply(clone, "initd.c", initd, dry)


RULES = {"gelee": rule_gelee}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("regle", choices=list(RULES))
    ap.add_argument("--clone", default=os.path.abspath(os.path.join(HERE, "..", "..")))
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    RULES[a.regle](a.clone, a.dry_run)
    akc.commit_all(a.clone, a.dry_run)


if __name__ == "__main__":
    sys.exit(main())
