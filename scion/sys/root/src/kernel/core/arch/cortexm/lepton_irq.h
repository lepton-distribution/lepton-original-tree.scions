/*
 * Lepton — sections critiques à nom neutre, Cortex-M (ARMv6-M, ARMv7-M). Licence : voir LICENSE
 * (MPL 1.1).
 *
 * ETAPE-3 tâche 1 : noms neutres (__lepton_disable_irq…) pour d'autres ISA (RISC-V : mstatus.MIE),
 * sélectionnés par l'axe ISA (chemin d'inclusion de cmake/isa/<isa>.cmake). Masquage par PRIMASK,
 * registre architectural d'ARMv6-M et d'ARMv7-M ; équivalents CMSIS-Core : __disable_irq,
 * __enable_irq, __get_PRIMASK, __set_PRIMASK. Écrits ici en assembleur en ligne pour ne pas
 * inclure core_cmFunc.h avant l'en-tête du cœur (__CORTEX_M, fourni par la carte).
 * Attention : sous embOS, préférer les primitives du micro-noyau (OS_IncDI/OS_DecRI, KAL) ; ces
 * macros masquent aussi les interruptions au-dessus du seuil d'embOS.
 */
#ifndef __LEPTON_IRQ_H__
#define __LEPTON_IRQ_H__

#include <stdint.h>
#include "kernel/core/compiler.h"

typedef uint32_t lepton_irq_state_t;

static inline void __lepton_disable_irq(void){
   __asm volatile ("cpsid i" : : : "memory");
}

static inline void __lepton_enable_irq(void){
   __asm volatile ("cpsie i" : : : "memory");
}

/* masque les interruptions et rend l'état précédent (sections critiques imbriquées) */
static inline lepton_irq_state_t __lepton_irq_save(void){
   lepton_irq_state_t primask;
   __asm volatile ("mrs %0, primask\n\tcpsid i" : "=r"(primask) : : "memory");
   return primask;
}

static inline void __lepton_irq_restore(lepton_irq_state_t primask){
   __asm volatile ("msr primask, %0" : : "r"(primask) : "memory");
}

#endif
