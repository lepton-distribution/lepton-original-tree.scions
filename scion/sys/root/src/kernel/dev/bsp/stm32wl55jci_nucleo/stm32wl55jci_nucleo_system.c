/*
 * Lepton — BSP de la carte NUCLEO-WL55JC1 (STM32WL55JC, Cortex-M4 du CPU1, étape 6).
 * Licence : voir LICENSE (MPL 1.1).
 *
 * Horloges (SystemInit, appelée par le démarrage générique kernel/core/arch/cortexm après
 * l'initialisation de .data et .bss) : avant le micro-noyau, qui calcule son tick sur
 * SystemCoreClock (le portage IAR changeait l'horloge au chargement du pilote cpu0, après
 * OS_InitHW, et réarmait le SysTick par HAL_Init : pilote non repris).
 * Interruptions : IRQ<n>_Handler de la table générique, reliés aux gestionnaires nommés des
 * pilotes du portage IAR : USART2 (dev_stm32wl55jci_nucleo_usart_2.c ; DMA1_Channel1_IRQn = 11,
 * DMA1_Channel2_IRQn = 12, USART2_IRQn = 37) et radio (SUBGHZ_Radio_IRQn = 50) (stm32wl55xx.h,
 * CORE_CM4). Le pilote USART2 active ses interruptions sans régler leur priorité (0 : hors de la
 * plage d'embOS, alors qu'il appelle le noyau) : priorités posées ici ; le pilote radio règle la
 * sienne (HAL_SUBGHZ_MspInit, 0xC0). Le CPU2 (Cortex-M0+) n'est jamais démarré (PWR_CR4.C2BOOT à 0).
 * Valeurs : stm32wl55jci_nucleo.h.
 */
#include <stdint.h>

#include "stm32wlxx.h"
#include "stm32wl55jci_nucleo.h"

/* --- variables et tables de system_stm32wlxx.c (CMSIS Device ST, non compilé : il définit
 *     SystemInit), lues par la HAL RCC ; valeurs reprises sans modification ------------------ */
uint32_t SystemCoreClock = 4000000u;   /* MSI 4 MHz après reset */
const uint32_t AHBPrescTable[16u] = {1u, 3u, 5u, 1u, 1u, 6u, 10u, 32u, 2u, 4u, 8u, 16u, 64u,
                                     128u, 256u, 512u};
const uint32_t APBPrescTable[8u] = {0u, 0u, 0u, 0u, 1u, 2u, 3u, 4u};
const uint32_t MSIRangeTable[16u] = {100000u, 200000u, 400000u, 800000u, 1000000u, 2000000u,
                                     4000000u, 8000000u, 16000000u, 24000000u, 32000000u,
                                     48000000u, 0u, 0u, 0u, 0u};

/* étape de SystemInit en échec (lisible au débogueur, palier 1) : 0 aucune, 1 échelle de
   tension, 2 latence flash, 3 MSI */
volatile uint32_t stm32wl55jci_nucleo_clock_error;

#define STM32WL55JCI_NUCLEO_WAIT_LOOPS 2000000u

static void _stm32wl55jci_nucleo_wait(volatile uint32_t* reg, uint32_t mask, uint32_t value,
                                      uint32_t step){
   uint32_t n;
   for(n = 0; n < STM32WL55JCI_NUCLEO_WAIT_LOOPS; n++) {
      if((*reg & mask) == value)
         return;
   }
   /* horloge défaillante : arrêt visible (le débit série serait faux) */
   stm32wl55jci_nucleo_clock_error = step;
   for(;;) {
   }
}

void SystemInit(void){
   /* échelle de tension 1 (valeur de reset, requise au-delà de 16 MHz) */
   PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | PWR_CR1_VOS_0;
   _stm32wl55jci_nucleo_wait(&PWR->SR2, PWR_SR2_VOSF, 0, 1);

   /* états d'attente avant d'augmenter la fréquence ; caches et prefetch de la flash */
   FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY)
              | (STM32WL55JCI_NUCLEO_FLASH_LATENCY << FLASH_ACR_LATENCY_Pos)
              | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
   _stm32wl55jci_nucleo_wait(&FLASH->ACR, FLASH_ACR_LATENCY,
                             STM32WL55JCI_NUCLEO_FLASH_LATENCY << FLASH_ACR_LATENCY_Pos, 2);

   /* MSI (horloge système après reset) : range 11 = 48 MHz, sélectionné par MSIRGSEL ; MSIRANGE
      modifiable quand le MSI est prêt (RM0453 §6.4.1). Prédiviseurs AHB, HCLK3, APB : /1 (reset). */
   _stm32wl55jci_nucleo_wait(&RCC->CR, RCC_CR_MSIRDY, RCC_CR_MSIRDY, 3);
   RCC->CR = (RCC->CR & ~RCC_CR_MSIRANGE) | RCC_CR_MSIRANGE_11 | RCC_CR_MSIRGSEL;
   _stm32wl55jci_nucleo_wait(&RCC->CR, RCC_CR_MSIRDY, RCC_CR_MSIRDY, 3);
   SystemCoreClock = STM32WL55JCI_NUCLEO_SYSCLK_HZ;

   /* priorités des interruptions du pilote USART2 (activées par le pilote à l'ouverture) */
   NVIC_SetPriority(DMA1_Channel1_IRQn, STM32WL55JCI_NUCLEO_IRQ_PRIO);
   NVIC_SetPriority(DMA1_Channel2_IRQn, STM32WL55JCI_NUCLEO_IRQ_PRIO);
   NVIC_SetPriority(USART2_IRQn, STM32WL55JCI_NUCLEO_IRQ_PRIO);
}

void SystemCoreClockUpdate(void){
   /* seule source ici : MSI, réglé par SystemInit (48 MHz) ou range de reset */
   SystemCoreClock = MSIRangeTable[(RCC->CR & RCC_CR_MSIRANGE) >> RCC_CR_MSIRANGE_Pos];
}

/* --- interruptions : gestionnaires nommés des pilotes USART2 et radio (portage IAR) ----------- */
extern void DMA1_Channel1_IRQHandler(void);
extern void DMA1_Channel2_IRQHandler(void);
extern void USART2_IRQHandler(void);
extern void SUBGHZ_Radio_IRQHandler(void);

void IRQ11_Handler(void) { DMA1_Channel1_IRQHandler(); }
void IRQ12_Handler(void) { DMA1_Channel2_IRQHandler(); }
void IRQ37_Handler(void) { USART2_IRQHandler(); }
void IRQ50_Handler(void) { SUBGHZ_Radio_IRQHandler(); }
