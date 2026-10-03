/*
 * Lepton — vecteurs d'interruption du STM32F429/F439 (NUCLEO-F439ZI, étape 5).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Le démarrage générique ARMv7-M (kernel/core/arch/cortexm/startup_armv7m.c) fournit la table
 * des exceptions et des IRQ 0 à 63 sous les noms IRQ<n>_Handler (faibles). Ce fichier :
 *  - définit les IRQ<n>_Handler (n < 64) utilisés par la carte, vers les gestionnaires aux noms
 *    CMSIS (USART3_IRQHandler…) des pilotes STM32F4 et du BSP ;
 *  - prolonge la table pour les IRQ 64 à 90 du STM32F42x/43x (section .isr_vector_ext, placée
 *    juste après .isr_vector par ld/common-cortexm.ld).
 * Numéros : RM0090, table 62 (stm32f429xx.h, IRQn_Type).
 */
#include <stdint.h>

void Default_Handler(void);   /* startup_armv7m.c */

void USART3_IRQHandler(void);
void DMA1_Stream1_IRQHandler(void);
void USART6_IRQHandler(void);
void DMA2_Stream1_IRQHandler(void);
void ETH_IRQHandler(void);

void IRQ12_Handler(void) { DMA1_Stream1_IRQHandler(); }   /* DMA1_Stream1_IRQn */
void IRQ39_Handler(void) { USART3_IRQHandler(); }         /* USART3_IRQn */
void IRQ57_Handler(void) { DMA2_Stream1_IRQHandler(); }   /* DMA2_Stream1_IRQn */
void IRQ61_Handler(void) { ETH_IRQHandler(); }            /* ETH_IRQn */

#define NUCLEO_F439ZI_IRQ_FIRST_EXT  64
#define NUCLEO_F439ZI_IRQ_COUNT      91

typedef void (*nucleo_f439zi_vector_t)(void);

__attribute__((section(".isr_vector_ext"), used))
const nucleo_f439zi_vector_t nucleo_f439zi_vector_table_ext[NUCLEO_F439ZI_IRQ_COUNT
                                                            - NUCLEO_F439ZI_IRQ_FIRST_EXT] = {
   [0 ... (NUCLEO_F439ZI_IRQ_COUNT - NUCLEO_F439ZI_IRQ_FIRST_EXT - 1)] = Default_Handler,
   [71 - NUCLEO_F439ZI_IRQ_FIRST_EXT] = USART6_IRQHandler,                /* USART6_IRQn */
};
