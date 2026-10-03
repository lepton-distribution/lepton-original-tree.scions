/*
 * Lepton — démarrage ARMv7-M (Cortex-M3/M4/M7), GCC. Licence : voir LICENSE (MPL 1.1).
 *
 * Écrit pour la migration GCC/Linux (étape 3) : table des vecteurs, copie de .data, mise à zéro
 * de .bss et de .ccm_bss (étape 5), activation de la FPU si le code l'utilise, constructeurs,
 * main(). Commun à toutes les cartes ARMv7-M : les numéros d'interruption externes sont associés à leurs pilotes par le BSP de
 * la carte, qui définit les IRQ<n>_Handler dont il a besoin (les autres restent sur
 * Default_Handler). PendSV_Handler vient du micro-noyau (bibliothèque embOS), SysTick_Handler de
 * son intégration (core-<backend>/arch/armv7m).
 * Symboles du script de liens : ld/common-cortexm.ld.
 */
#include <stdint.h>

extern uint32_t __stack_top__;
extern uint32_t __data_load__, __data_start__, __data_end__;
extern uint32_t __bss_start__, __bss_end__;
/* seconde zone de .bss (données du CPU seul : CCM du STM32F4, sinon RAM ou vide), ld/common-cortexm.ld */
extern uint32_t __ccm_bss_start__, __ccm_bss_end__;

extern void SystemInit(void);
extern void __libc_init_array(void);
extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

#define LEPTON_WEAK_HANDLER(name) void name(void) __attribute__((weak, alias("Default_Handler")))

LEPTON_WEAK_HANDLER(NMI_Handler);
LEPTON_WEAK_HANDLER(HardFault_Handler);
LEPTON_WEAK_HANDLER(MemManage_Handler);
LEPTON_WEAK_HANDLER(BusFault_Handler);
LEPTON_WEAK_HANDLER(UsageFault_Handler);
LEPTON_WEAK_HANDLER(SVC_Handler);
LEPTON_WEAK_HANDLER(DebugMon_Handler);
LEPTON_WEAK_HANDLER(PendSV_Handler);
LEPTON_WEAK_HANDLER(SysTick_Handler);

#define LEPTON_IRQ_HANDLERS_8(b) \
   LEPTON_WEAK_HANDLER(IRQ##b##0_Handler); LEPTON_WEAK_HANDLER(IRQ##b##1_Handler); \
   LEPTON_WEAK_HANDLER(IRQ##b##2_Handler); LEPTON_WEAK_HANDLER(IRQ##b##3_Handler); \
   LEPTON_WEAK_HANDLER(IRQ##b##4_Handler); LEPTON_WEAK_HANDLER(IRQ##b##5_Handler); \
   LEPTON_WEAK_HANDLER(IRQ##b##6_Handler); LEPTON_WEAK_HANDLER(IRQ##b##7_Handler)

/* IRQ0..IRQ63 : noms IRQ<n>_Handler (n décimal, sans zéro de tête). */
LEPTON_WEAK_HANDLER(IRQ0_Handler);  LEPTON_WEAK_HANDLER(IRQ1_Handler);
LEPTON_WEAK_HANDLER(IRQ2_Handler);  LEPTON_WEAK_HANDLER(IRQ3_Handler);
LEPTON_WEAK_HANDLER(IRQ4_Handler);  LEPTON_WEAK_HANDLER(IRQ5_Handler);
LEPTON_WEAK_HANDLER(IRQ6_Handler);  LEPTON_WEAK_HANDLER(IRQ7_Handler);
LEPTON_WEAK_HANDLER(IRQ8_Handler);  LEPTON_WEAK_HANDLER(IRQ9_Handler);
LEPTON_IRQ_HANDLERS_8(1); LEPTON_WEAK_HANDLER(IRQ18_Handler); LEPTON_WEAK_HANDLER(IRQ19_Handler);
LEPTON_IRQ_HANDLERS_8(2); LEPTON_WEAK_HANDLER(IRQ28_Handler); LEPTON_WEAK_HANDLER(IRQ29_Handler);
LEPTON_IRQ_HANDLERS_8(3); LEPTON_WEAK_HANDLER(IRQ38_Handler); LEPTON_WEAK_HANDLER(IRQ39_Handler);
LEPTON_IRQ_HANDLERS_8(4); LEPTON_WEAK_HANDLER(IRQ48_Handler); LEPTON_WEAK_HANDLER(IRQ49_Handler);
LEPTON_IRQ_HANDLERS_8(5); LEPTON_WEAK_HANDLER(IRQ58_Handler); LEPTON_WEAK_HANDLER(IRQ59_Handler);
LEPTON_WEAK_HANDLER(IRQ60_Handler); LEPTON_WEAK_HANDLER(IRQ61_Handler);
LEPTON_WEAK_HANDLER(IRQ62_Handler); LEPTON_WEAK_HANDLER(IRQ63_Handler);

#define V(n) IRQ##n##_Handler

typedef void (*lepton_vector_t)(void);

__attribute__((section(".isr_vector"), used))
const lepton_vector_t lepton_vector_table[16 + 64] = {
   (lepton_vector_t)&__stack_top__,
   Reset_Handler,
   NMI_Handler,
   HardFault_Handler,
   MemManage_Handler,
   BusFault_Handler,
   UsageFault_Handler,
   0, 0, 0, 0,
   SVC_Handler,
   DebugMon_Handler,
   0,
   PendSV_Handler,
   SysTick_Handler,
   V(0),  V(1),  V(2),  V(3),  V(4),  V(5),  V(6),  V(7),  V(8),  V(9),
   V(10), V(11), V(12), V(13), V(14), V(15), V(16), V(17), V(18), V(19),
   V(20), V(21), V(22), V(23), V(24), V(25), V(26), V(27), V(28), V(29),
   V(30), V(31), V(32), V(33), V(34), V(35), V(36), V(37), V(38), V(39),
   V(40), V(41), V(42), V(43), V(44), V(45), V(46), V(47), V(48), V(49),
   V(50), V(51), V(52), V(53), V(54), V(55), V(56), V(57), V(58), V(59),
   V(60), V(61), V(62), V(63),
};

/* Registre architectural ARMv7-M (SCB->CPACR). */
#define LEPTON_SCB_CPACR (*(volatile uint32_t*)0xE000ED88u)

void Reset_Handler(void){
   uint32_t* src = &__data_load__;
   uint32_t* dst = &__data_start__;

   while(dst < &__data_end__)
      *dst++ = *src++;
   for(dst = &__bss_start__; dst < &__bss_end__; dst++)
      *dst = 0;
   for(dst = &__ccm_bss_start__; dst < &__ccm_bss_end__; dst++)
      *dst = 0;

#if defined(__ARM_FP)
   /* FPU (CP10, CP11) accessible avant la première instruction flottante. */
   LEPTON_SCB_CPACR |= (0xFu << 20);
   __asm volatile ("dsb\n\tisb" ::: "memory");
#endif

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
