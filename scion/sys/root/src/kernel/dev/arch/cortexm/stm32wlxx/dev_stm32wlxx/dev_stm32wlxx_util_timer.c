/*
 * Lepton — base de temps des utilitaires STM32CubeWL pour la pile radio (famille STM32WLxx,
 * étape 6). Licence : voir LICENSE (MPL 1.1).
 *
 * SubGHz_Phy (radio.c, écoute avant émission) mesure des durées en millisecondes par
 * TimerGetCurrentTime / TimerGetElapsedTime, ramenés par stm32_radio_target/timer.h à
 * UTIL_TIMER_GetCurrentTime / UTIL_TIMER_GetElapsedTime (Utilities/timer/stm32_timer.c, non
 * compilé : serveur de minuteries sur RTC, inutile ici ; les minuteries de la pile sont neutralisées
 * par timer.h). Seules ces deux fonctions sont fournies, sur le tick du noyau (comme HAL_GetTick).
 */
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/systime.h"

#include "stm32_timer.h"

UTIL_TIMER_Time_t UTIL_TIMER_GetCurrentTime(void){
   return (UTIL_TIMER_Time_t)((uint64_t)__kernel_get_timer_ticks() * 1000u / HZ);
}

UTIL_TIMER_Time_t UTIL_TIMER_GetElapsedTime(UTIL_TIMER_Time_t past){
   return UTIL_TIMER_GetCurrentTime() - past;
}
