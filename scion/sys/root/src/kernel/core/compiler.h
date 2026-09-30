/*
 * Lepton — extensions du compilateur (macros __lepton_*). Licence : voir LICENSE (MPL 1.1).
 *
 * Seul point d'accès du code Lepton aux extensions du compilateur (CLAUDE.md, ETAPE-3 tâche 1,
 * ETAPE-4 tâche 1) : GCC seul (IAR n'est plus supporté, décision 2026-09-30). La couche de macros
 * isole le code d'un changement futur de compilateur. Les sections critiques, qui dépendent du
 * processeur, sont dans l'axe ISA (kernel/core/arch/<isa>/lepton_irq.h).
 */
#ifndef __LEPTON_COMPILER_H__
#define __LEPTON_COMPILER_H__

#if !defined(__GNUC__)
   #error "Lepton : compilateur non supporté (GCC seul, kernel/core/compiler.h)"
#endif

/* variable non initialisée au démarrage (conservée au reset logiciel) */
#define __lepton_no_init            __attribute__((section(".noinit")))
/* symbole conservé par l'édition de liens (ex-__root IAR) */
#define __lepton_used               __attribute__((used))
#define __lepton_weak               __attribute__((weak))
#define __lepton_packed             __attribute__((packed))
#define __lepton_align(__n__)       __attribute__((aligned(__n__)))
#define __lepton_section(__s__)     __attribute__((section(__s__)))
/* fonction exécutée depuis la RAM (ex-__ramfunc IAR) */
#define __lepton_ramfunc            __attribute__((section(".ramfunc"), long_call))
#define __lepton_noreturn           __attribute__((noreturn))

#endif
