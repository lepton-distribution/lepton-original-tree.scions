/*
 * Lepton — crochets de FreeRTOS exigés par la configuration (étape 7). Licence : voir LICENSE
 * (MPL 1.1). Équivalent d'OS_Error (core-segger/arch/armv7m/embos_init_hw.c) : la cause est
 * mémorisée dans des variables lisibles au débogueur, puis arrêt interruptions masquées.
 *   lepton_freertos_assert()        : configASSERT (FreeRTOSConfig.h) ;
 *   vApplicationStackOverflowHook() : configCHECK_FOR_STACK_OVERFLOW = 2.
 * Pas de dépendance au matériel : commun à ARMv7-M et ARMv6-M (SysTick géré par le port).
 */
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

volatile const char* lepton_freertos_assert_file;
volatile int lepton_freertos_assert_line;
volatile const char* lepton_freertos_overflow_task;

void lepton_freertos_assert(const char* file, int line){
   portDISABLE_INTERRUPTS();
   lepton_freertos_assert_file = file;
   lepton_freertos_assert_line = line;
   for(;;) {
   }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char* name){
   (void)task;
   portDISABLE_INTERRUPTS();
   lepton_freertos_overflow_task = name;
   for(;;) {
   }
}
