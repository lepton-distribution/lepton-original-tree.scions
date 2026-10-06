/*
 * Lepton — point d'entrée FreeRTOS sur ARMv7-M (étape 7). Licence : voir LICENSE (MPL 1.1).
 * Même séquence que core-segger/arch/armv7m/embos_main.c : interruptions masquées, noyau Lepton
 * (_start_kernel : tâche noyau et amorçage), démarrage du multitâche. FreeRTOS n'a pas
 * d'initialisation séparée ; SysTick, PendSV et SVC sont configurés par xPortStartScheduler
 * (priorités : FreeRTOSConfig.h). Avant le démarrage, les sections critiques de l'API laissent
 * les interruptions masquées (uxCriticalNesting initial), comme OS_IncDI pour embOS.
 */
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

extern void _start_kernel(char* arg);

int main(void){
   portDISABLE_INTERRUPTS();   /* interruptions masquées jusqu'au démarrage de l'ordonnanceur */
   _start_kernel(0);           /* noyau Lepton : tâche noyau, rootfs, pilotes, boot */
   vTaskStartScheduler();      /* multitâche ; ne revient pas (mémoire statique) */
   for(;;) {
   }
   return 0;
}
