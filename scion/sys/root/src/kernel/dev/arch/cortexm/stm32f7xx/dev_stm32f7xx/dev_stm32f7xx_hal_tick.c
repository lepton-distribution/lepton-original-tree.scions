/*
 * Lepton — base de temps de la HAL STM32F7 (famille STM32F7xx, étape 6). Licence : voir LICENSE (MPL 1.1).
 *
 * La HAL ST mesure ses délais d'attente en millisecondes par HAL_GetTick (faible dans
 * stm32f7xx_hal.c, non lié : le SysTick appartient au micro-noyau). Lepton fournit le tick du
 * noyau (__kernel_get_timer_ticks, kernel/core/interrupt.h, défini par le micro-noyau) ramené
 * en millisecondes : indépendant d'embOS ou de FreeRTOS. Avant le démarrage de l'ordonnanceur,
 * le tick n'avance pas : n'appeler la HAL qu'en contexte de processus (ouverture d'un pilote).
 */
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/systime.h"

#include "stm32f7xx_hal.h"

uint32_t HAL_GetTick(void){
   return (uint32_t)((uint64_t)__kernel_get_timer_ticks() * 1000u / HZ);
}

void HAL_Delay(uint32_t Delay){
   uint32_t start = HAL_GetTick();
   /* sémantique de la HAL : au moins Delay ms (+1 pour le tick en cours) */
   while((HAL_GetTick() - start) <= Delay) {
   }
}
