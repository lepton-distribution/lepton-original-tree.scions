/*
 * Lepton — base de temps de la HAL STM32WL (famille STM32WLxx, étape 6). Licence : voir LICENSE (MPL 1.1).
 *
 * Comme dev_stm32f7xx_hal_tick.c : la HAL ST mesure ses délais d'attente en millisecondes par
 * HAL_GetTick (faible dans stm32wlxx_hal.c, non lié : le SysTick appartient au micro-noyau, HAL_Init
 * n'est pas appelée). Tick du noyau ramené en millisecondes. Le portage IAR (pilote cpu0, non
 * repris) renvoyait le tick brut et attendait par __kernel_usleep(Delay) en microsecondes.
 * Avant le démarrage de l'ordonnanceur, le tick n'avance pas : n'appeler la HAL qu'en contexte
 * de processus (ouverture d'un pilote).
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
   uint32_t start = HAL_GetTick();
   /* sémantique de la HAL : au moins Delay ms (+1 pour le tick en cours) */
   while((HAL_GetTick() - start) <= Delay) {
   }
}
