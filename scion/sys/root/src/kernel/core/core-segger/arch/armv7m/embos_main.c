/*
 * Lepton — point d'entrée embOS sur ARMv7-M. Licence : voir LICENSE (MPL 1.1).
 * Séquence reprise du main Lepton de l'intégration IAR (ucore/embOSCXM4_518/arch/cmsis/main.c) :
 * interruptions masquées, noyau embOS, matériel du micro-noyau, noyau Lepton (_start_kernel :
 * tâche noyau et amorçage), démarrage du multitâche. OS_Init remplace OS_InitKern (alias V5).
 */
#include <stdint.h>
#include "RTOS.h"

extern void _start_kernel(char* arg);

int main(void){
   OS_IncDI();          /* interruptions masquées jusqu'à OS_Start */
   OS_Init();           /* noyau embOS */
   OS_InitHW();         /* SysTick, priorités (embos_init_hw.c) */
   _start_kernel(0);    /* noyau Lepton : tâche noyau, rootfs, pilotes, boot */
   OS_Start();          /* multitâche ; ne revient pas */
   return 0;
}
