/*
 * Lepton — démarrage ARMv6-M (Cortex-M0/M0+), GCC. Licence : voir LICENSE (MPL 1.1).
 *
 * Écrit pour la migration GCC/Linux (étape 6), sur le modèle de startup_armv7m.c : table des
 * vecteurs ARMv6-M (pas de MemManage, BusFault, UsageFault ni DebugMon ; 32 interruptions
 * externes au plus, ARMv6-M ARM B3.4), copie de .data, mise à zéro de .bss et de .ccm_bss,
 * constructeurs, main(). Pas de FPU. Commun à toutes les cartes ARMv6-M : le BSP définit les
 * IRQ<n>_Handler dont il a besoin. PendSV_Handler vient du micro-noyau (bibliothèque embOS),
 * SysTick_Handler de son intégration (core-<backend>/arch/armv6m).
 * Symboles du script de liens : ld/common-cortexm.ld.
 */
#include <stdint.h>

extern uint32_t __stack_top__;
extern uint32_t __data_load__, __data_start__, __data_end__;
extern uint32_t __bss_start__, __bss_end__;
/* seconde zone de .bss (ld/common-cortexm.ld) : en RAM ou vide sur les cartes ARMv6-M */
extern uint32_t __ccm_bss_start__, __ccm_bss_end__;

extern void SystemInit(void);
extern void __libc_init_array(void);
extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

#define LEPTON_WEAK_HANDLER(name) void name(void) __attribute__((weak, alias("Default_Handler")))

LEPTON_WEAK_HANDLER(NMI_Handler);
LEPTON_WEAK_HANDLER(HardFault_Handler);
LEPTON_WEAK_HANDLER(SVC_Handler);
LEPTON_WEAK_HANDLER(PendSV_Handler);
LEPTON_WEAK_HANDLER(SysTick_Handler);

#define LEPTON_IRQ_HANDLERS_8(b) \
   LEPTON_WEAK_HANDLER(IRQ##b##0_Handler); LEPTON_WEAK_HANDLER(IRQ##b##1_Handler); \
   LEPTON_WEAK_HANDLER(IRQ##b##2_Handler); LEPTON_WEAK_HANDLER(IRQ##b##3_Handler); \
   LEPTON_WEAK_HANDLER(IRQ##b##4_Handler); LEPTON_WEAK_HANDLER(IRQ##b##5_Handler); \
   LEPTON_WEAK_HANDLER(IRQ##b##6_Handler); LEPTON_WEAK_HANDLER(IRQ##b##7_Handler)

/* IRQ0..IRQ31 : noms IRQ<n>_Handler (n décimal, sans zéro de tête). */
LEPTON_WEAK_HANDLER(IRQ0_Handler);  LEPTON_WEAK_HANDLER(IRQ1_Handler);
LEPTON_WEAK_HANDLER(IRQ2_Handler);  LEPTON_WEAK_HANDLER(IRQ3_Handler);
LEPTON_WEAK_HANDLER(IRQ4_Handler);  LEPTON_WEAK_HANDLER(IRQ5_Handler);
LEPTON_WEAK_HANDLER(IRQ6_Handler);  LEPTON_WEAK_HANDLER(IRQ7_Handler);
LEPTON_WEAK_HANDLER(IRQ8_Handler);  LEPTON_WEAK_HANDLER(IRQ9_Handler);
LEPTON_IRQ_HANDLERS_8(1); LEPTON_WEAK_HANDLER(IRQ18_Handler); LEPTON_WEAK_HANDLER(IRQ19_Handler);
LEPTON_IRQ_HANDLERS_8(2); LEPTON_WEAK_HANDLER(IRQ28_Handler); LEPTON_WEAK_HANDLER(IRQ29_Handler);
LEPTON_WEAK_HANDLER(IRQ30_Handler); LEPTON_WEAK_HANDLER(IRQ31_Handler);

#define V(n) IRQ##n##_Handler

typedef void (*lepton_vector_t)(void);

__attribute__((section(".isr_vector"), used))
const lepton_vector_t lepton_vector_table[16 + 32] = {
   (lepton_vector_t)&__stack_top__,
   Reset_Handler,
   NMI_Handler,
   HardFault_Handler,
   0, 0, 0, 0, 0, 0, 0,
   SVC_Handler,
   0, 0,
   PendSV_Handler,
   SysTick_Handler,
   V(0),  V(1),  V(2),  V(3),  V(4),  V(5),  V(6),  V(7),  V(8),  V(9),
   V(10), V(11), V(12), V(13), V(14), V(15), V(16), V(17), V(18), V(19),
   V(20), V(21), V(22), V(23), V(24), V(25), V(26), V(27), V(28), V(29),
   V(30), V(31),
};

void Reset_Handler(void){
   uint32_t* src = &__data_load__;
   uint32_t* dst = &__data_start__;

   while(dst < &__data_end__)
      *dst++ = *src++;
   for(dst = &__bss_start__; dst < &__bss_end__; dst++)
      *dst = 0;
   for(dst = &__ccm_bss_start__; dst < &__ccm_bss_end__; dst++)
      *dst = 0;

   SystemInit();
   __libc_init_array();
   main();
   for(;;) {
   }
}

void Default_Handler(void){
   for(;;) {
   }
}
