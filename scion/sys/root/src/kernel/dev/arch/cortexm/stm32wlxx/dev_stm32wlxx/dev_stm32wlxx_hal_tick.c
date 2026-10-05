/*
 * Lepton — base de temps de la HAL STM32WL (famille STM32WLxx, étape 6). Licence : voir LICENSE (MPL 1.1).
 *
 * Comme dev_stm32f7xx_hal_tick.c : la HAL ST mesure ses délais d'attente en millisecondes par
 * HAL_GetTick (faible dans stm32wlxx_hal.c : le SysTick appartient au micro-noyau, HAL_Init
 * n'est pas appelée). Tick du noyau ramené en millisecondes. Le portage IAR (pilote cpu0, non
 * repris) renvoyait le tick brut.
 * HAL_Delay attend sur le compteur de cycles DWT du Cortex-M4, et non sur le tick : la pile radio
 * l'appelle (RADIO_DELAY_MS) au chargement des pilotes, interruptions masquées jusqu'à OS_Start,
 * tick figé. Attente active, sans rendre la main : réservée aux délais courts des pilotes ST.
 * Fichier objet de l'exécutable (cmake/boards) : dans une bibliothèque, les définitions faibles de
 * stm32wlxx_hal.c suffiraient à l'éditeur de liens et celles-ci ne seraient pas tirées.
 */
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/systime.h"

#include "stm32wlxx_hal.h"

uint32_t HAL_GetTick(void){
   return (uint32_t)((uint64_t)__kernel_get_timer_ticks() * 1000u / HZ);
}

void HAL_Delay(uint32_t Delay){
   uint32_t cycles_ms = SystemCoreClock / 1000u;
   uint32_t start;
   /* compteur de cycles : traçage activé (DEMCR.TRCENA), compteur lancé une fois */
   if(!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk)) {
      CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
      DWT->CYCCNT = 0;
      DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
   }
   /* sémantique de la HAL : au moins Delay ms (+1 pour le tick en cours) ; une milliseconde à la
      fois (le compteur de 32 bits reboucle en 89 s à 48 MHz) */
   for(Delay++; Delay > 0; Delay--) {
      start = DWT->CYCCNT;
      while((DWT->CYCCNT - start) < cycles_ms) {
      }
   }
}
