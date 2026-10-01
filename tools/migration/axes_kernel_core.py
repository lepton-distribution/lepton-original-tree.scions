#!/usr/bin/env python3
"""axes_kernel_core.py — directives d'ISA/cœur de kernel/core hors KAL (module KAL-2, ETAPE-4
tâche 3), règles mécaniques rejouables à contenu constant (gcc -E -P identique).

Opère sur le clone (scion/…), jamais sur le trunk ; lecture/écriture octet pour octet (Latin-1,
fins de ligne conservées). Les branches sont repérées par leur condition (analyseur de
kal_split.py). Chaque règle vérifie ses préconditions et s'arrête si elle est déjà appliquée.

Règles (une par commit, dans cet ordre) :
  statique   CPU_GNU32 → USE_KERNEL_STATIC là où la condition porte sur le micro-noyau :
             malloc.c (sections atomiques, ×8), kernel.h (__mk_syscall du noyau statique).
             Sur toutes les configurations, CPU_GNU32 n'est posé qu'avec USE_KERNEL_STATIC.
  gelee      branches de cibles gelées retirées (D3a étendu à la simulation Linux, 2026-10-01),
             copie d'origine sous scion/legacy/ si absente :
               kernel.h      __mk_syscall eCos Cortex-M (CPU_CORTEXM sans micro-noyau),
                             prototypes de la simulation Linux ;
               kernelconf.h  kernel_mkconf.h de la simulation x86 (arch/synthetic, absent) ;
               ethif_core.c  adresse MAC de la simulation Linux (CPU_GNU32) ;
               timer.h       _SC_CLK_TCK 1000 des cibles ni hôte ni Cortex-M (ARM7, M16C).
  inutilise  kernel_pthread.h : KERNEL_STACK (défini, jamais utilisé) ; kernelconf.h :
             __KERNEL_POSIX_REALTIME_SIGNALS redéfini à l'identique dans le bloc FreeRTOS ;
             kernel.h : prototypes do_swi / _kernel_syscall_handler (eCos), jamais définis.
             gcc -E : seules ces deux déclarations disparaissent (cibles Cortex-M).

Usage : axes_kernel_core.py <règle> [--clone <racine du clone>] [--dry-run]
"""
import argparse
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import kal_split as ks  # noqa: E402

CORE = "sys/root/src/kernel/core"


def path(clone, rel):
    return os.path.join(clone, "scion", CORE, rel)


def read(clone, rel):
    with open(path(clone, rel), encoding="latin-1", newline="") as f:
        return f.read().splitlines(keepends=True)


def write(clone, rel, lines, dry):
    if dry:
        print("  (dry-run) écrirait %s" % rel)
        return
    with open(path(clone, rel), "w", encoding="latin-1", newline="") as f:
        f.writelines(lines)


def legacy(clone, rel, dry):
    dst = os.path.join(clone, "scion", "legacy", CORE, rel)
    if os.path.exists(dst):
        return
    if not dry:
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copy2(path(clone, rel), dst)
    print("  copie d'origine : legacy/%s/%s" % (CORE, rel))


def remove(lines, conds, label):
    with_cond = lambda c, g, k: c in conds
    lines, n = ks.remove_all(lines, with_cond, None, label)
    return lines, n


def keep_first_arm(lines, cond, label):
    """Garde le corps du premier bras (condition cond) et retire les autres : inconditionnel."""
    for g in ks.parse(lines):
        if g.arms[0][3] == cond:
            start, end = g.arm_body(0)
            print("  %-48s 1" % label)
            return lines[:g.arms[0][0]] + lines[start:end] + lines[g.endif[1]:], 1
    return lines, 0


def replace_exact(lines, old, new, label):
    text = "".join(lines)
    n = text.count(old)
    if n != 1:
        return lines, 0
    print("  %-48s 1" % label)
    return text.replace(old, new).splitlines(keepends=True), 1


def apply(clone, rel, fn, dry, need_legacy=False):
    """Calcule la transformation de rel ; rien n'est écrit avant que toute la règle ait réussi."""
    lines = read(clone, rel)
    new, n = fn(lines)
    if not n:
        raise SystemExit("%s : rien à faire (règle déjà appliquée ?) ; aucun fichier écrit" % rel)
    PENDING.append((rel, new, need_legacy))


PENDING = []


def commit_all(clone, dry):
    for rel, new, need_legacy in PENDING:
        if need_legacy:
            legacy(clone, rel, dry)
        write(clone, rel, new, dry)


# --- règle statique -------------------------------------------------------------------------

def rule_statique(clone, dry):
    def malloc(lines):
        out = [l.replace("#if !defined(CPU_GNU32)", "#if !defined(USE_KERNEL_STATIC)", 1)
               if l.startswith("#if !defined(CPU_GNU32)") else l for l in lines]
        return out, sum(1 for a, b in zip(lines, out) if a != b)

    def kernel_h(lines):
        out = [l.replace("#elif defined(CPU_GNU32)", "#elif defined(USE_KERNEL_STATIC)", 1)
               if l.rstrip("\r\n") == "#elif defined(CPU_GNU32)" else l for l in lines]
        return out, sum(1 for a, b in zip(lines, out) if a != b)

    print("malloc.c")
    apply(clone, "malloc.c", malloc, dry)
    print("kernel.h")
    apply(clone, "kernel.h", kernel_h, dry)


# --- règle gelee ----------------------------------------------------------------------------

MK_SYSCALL_GROUP = "defined(__KERNEL_UCORE_EMBOS) || defined(__KERNEL_UCORE_FREERTOS)"

KERNELCONF_MKCONF_OLD = """   #if defined(USE_KERNEL_STATIC)
      //for lepton as bootloader (no scheduler, static)
      //configuration fixe du noyau statique, trouvée par chemin d'inclusion (cmake/kal/static.cmake)
      #include "kernel_mkconf.h"
   #else
      #if defined(CPU_CORTEXM)
         //genere par mklepton dans le repertoire de build (cmake/mklepton.cmake), chemin d'inclusion
         #include "kernel_mkconf.h"
      #else
         #include "kernel/core/arch/synthetic/x86/kernel_mkconf.h"
      #endif
   #endif
"""
KERNELCONF_MKCONF_NEW = """   //kernel_mkconf.h, par chemin d'inclusion : configuration fixe du noyau statique
   //(cmake/kal/static.cmake, lepton comme bootloader sans ordonnanceur) ou generee par mklepton
   //dans le repertoire de build (cmake/mklepton.cmake).
   #include "kernel_mkconf.h"
"""


def rule_gelee(clone, dry):
    def kernel_h(lines):
        # __mk_syscall eCos Cortex-M : bras CPU_CORTEXM du groupe embOS/FreeRTOS/statique, jamais
        # actif ; prototypes de la simulation Linux. Les prototypes Cortex-M (do_swi…), actifs
        # mais jamais définis, relèvent de la règle inutilise (gcc -E différent).
        def pred(c, g, k):
            if c == "defined(CPU_GNU32) && !defined(USE_KERNEL_STATIC)":
                return True
            return c == "defined(CPU_CORTEXM)" and g.arms[0][3] == MK_SYSCALL_GROUP
        return ks.remove_all(lines, pred, None, "eCos Cortex-M, simulation Linux")

    def kernelconf(lines):
        # fichier Latin-1 dont ce bloc (ajouté à l'étape 2) est en UTF-8 : comparaison sur les
        # octets UTF-8 lus en Latin-1 ; le texte de remplacement est en ASCII
        old = KERNELCONF_MKCONF_OLD.encode("utf-8").decode("latin-1")
        return replace_exact(lines, old, KERNELCONF_MKCONF_NEW,
                             "kernel_mkconf.h de la simulation x86")

    def ethif(lines):
        return remove(lines, {"defined(CPU_GNU32)"}, "adresse MAC de la simulation Linux")

    def timer(lines):
        return keep_first_arm(lines, "defined(CPU_GNU32) || defined(CPU_CORTEXM)",
                              "_SC_CLK_TCK des cibles gelées")

    for rel, fn in (("kernel.h", kernel_h), ("kernelconf.h", kernelconf),
                    ("net/lwip_core/ethif_core.c", ethif), ("timer.h", timer)):
        print(rel)
        apply(clone, rel, fn, dry, need_legacy=True)


# --- règle inutilise ------------------------------------------------------------------------

def rule_inutilise(clone, dry):
    def pthread(lines):
        lines, n = keep_first_arm(lines, "defined(CPU_CORTEXM)", "KERNEL_STACK")
        if not n:
            return lines, 0
        # le premier bras gardé ne contient que la définition inutilisée : on la retire avec son
        # commentaire d'en-tête
        out, removed = [], 0
        for l in lines:
            s = l.strip()
            if s in ("//size of kernel stack", "#define KERNEL_STACK  2048"):
                removed += 1
                continue
            out.append(l)
        return out, removed == 2 and 1

    def kernelconf(lines):
        def pred(c, g, k):
            return (c == "(__tauon_cpu_core__ != __tauon_cpu_core_arm_cortexM0__)"
                    and g.parent is not None and g.parent.arms[0][3] == "defined(__KERNEL_UCORE_FREERTOS)")
        return ks.remove_all(lines, pred, None, "signaux temps réel redéfinis (FreeRTOS)")

    def kernel_h(lines):
        # prototypes do_swi / _kernel_syscall_handler (eCos) : jamais définis ni appelés
        return remove(lines, {"defined(CPU_CORTEXM)"}, "prototypes Cortex-M jamais définis")

    print("kernel_pthread.h")
    apply(clone, "kernel_pthread.h", pthread, dry)
    print("kernel.h")
    apply(clone, "kernel.h", kernel_h, dry)
    print("kernelconf.h")
    apply(clone, "kernelconf.h", kernelconf, dry)


RULES = {"statique": rule_statique, "gelee": rule_gelee, "inutilise": rule_inutilise}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("regle", choices=list(RULES))
    ap.add_argument("--clone", default=os.path.abspath(os.path.join(HERE, "..", "..")))
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    RULES[a.regle](a.clone, a.dry_run)
    commit_all(a.clone, a.dry_run)


if __name__ == "__main__":
    sys.exit(main())
