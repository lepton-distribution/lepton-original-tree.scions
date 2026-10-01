#!/usr/bin/env python3
"""axes_kernel_fs.py — directives d'ISA de kernel/fs (module kernel/fs, ETAPE-4 tâche 3), règles
mécaniques rejouables à contenu constant (gcc -E -P identique). Même mécanique que
axes_kernel_core.py (atomique, Latin-1 octet pour octet, analyseur de kal_split.py).

Règles (une par commit, dans cet ordre) :
  statique  rootfscore.c (×3) : « defined (WIN32) || defined(CPU_GNU32) » → defined(USE_KERNEL_STATIC)
            (tableaux initialisés du noyau statique hôte ; WIN32 = simulation gelée).
  gelee     branches de cibles gelées retirées (D3a), copie d'origine sous scion/legacy/ :
              ufs.c, ufsx.c  _ufs(x)_lookupdir des cibles ARM7 / Win32 ;
              fatcore.h      cache FAT de l'ARM9 et de la simulation Linux (FAT non compilé
                             sur l'hôte) ;
              fat16.c        garde !CPU_GNU32 de l'attribut .no_cache (simulation Linux).
  doublon   vfstypes.h : MAX_FILESYSTEM, deux branches identiques (CPU_CORTEXM ou non).

Usage : axes_kernel_fs.py <règle> [--clone <racine du clone>] [--dry-run]
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import axes_kernel_core as akc  # noqa: E402
import kal_split as ks  # noqa: E402

akc.CORE = "sys/root/src/kernel/fs"
apply, remove, keep_first_arm = akc.apply, akc.remove, akc.keep_first_arm

STATIQUE_OLD = "#if defined (WIN32) || defined(CPU_GNU32)"
STATIQUE_NEW = "#if defined(USE_KERNEL_STATIC)"


def rule_statique(clone, dry):
    def rootfs(lines):
        out = [l.replace(STATIQUE_OLD, STATIQUE_NEW, 1) if l.startswith(STATIQUE_OLD) else l
               for l in lines]
        return out, sum(1 for a, b in zip(lines, out) if a != b)

    print("rootfs/rootfscore.c")
    apply(clone, "rootfs/rootfscore.c", rootfs, dry)


def rule_gelee(clone, dry):
    def ufs(lines):
        return remove(lines, {"defined(CPU_ARM7) || defined(CPU_WIN32)"},
                      "_lookupdir ARM7 / Win32")

    def fatcore(lines):
        return remove(lines, {"defined(CPU_GNU32) || defined(CPU_ARM9)"},
                      "cache FAT ARM9 / simulation Linux")

    def fat16(lines):
        return keep_first_arm(lines, "!defined(CPU_GNU32)", "garde .no_cache (simulation Linux)")

    for rel, fn in (("ufs/ufs.c", ufs), ("ufs/ufsx.c", ufs), ("fat/fatcore.h", fatcore),
                    ("fat/fat16.c", fat16)):
        print(rel)
        apply(clone, rel, fn, dry, need_legacy=True)


def rule_doublon(clone, dry):
    def vfstypes(lines):
        # seul le groupe dont les deux bras sont identiques (MAX_FILESYSTEM)
        for g in ks.parse(lines):
            if g.arms[0][3] == "!defined(CPU_CORTEXM)" and len(g.arms) == 2:
                a, b = (lines[slice(*g.arm_body(k))] for k in (0, 1))
                if a == b:
                    print("  %-48s 1" % "MAX_FILESYSTEM (branches identiques)")
                    return lines[:g.arms[0][0]] + a + lines[g.endif[1]:], 1
        return lines, 0

    print("vfs/vfstypes.h")
    apply(clone, "vfs/vfstypes.h", vfstypes, dry)


RULES = {"statique": rule_statique, "gelee": rule_gelee, "doublon": rule_doublon}


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
