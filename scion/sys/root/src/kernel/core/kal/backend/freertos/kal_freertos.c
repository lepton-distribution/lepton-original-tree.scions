/*
 * Lepton — KAL, axe micro-noyau FreeRTOS : région atomique (étape 7). Licence : voir LICENSE
 * (MPL 1.1).
 *
 * __atomic_in/__atomic_out (kal_backend.h) = vTaskSuspendAll/xTaskResumeAll, comme
 * OS_EnterRegion/OS_LeaveRegion d'embOS : pas de commutation préemptive, interruptions actives.
 * Écart : embOS laisse une tâche bloquer dans une région (la région est quittée pendant
 * l'attente et reprise au réveil), FreeRTOS l'interdit (configASSERT : « Cannot block if the
 * scheduler is suspended »). Le noyau Lepton en dépend : kernel_io_write appelle le pilote dans
 * __atomic_in, et le pilote socket (lwIP) attend ses mutex et sémaphores. Constaté au palier
 * réseau QEMU (étape 7, ftpd : queue.c:1682).
 *
 * Toute attente bloquante du backend passe donc par __kal_frt_block (kal_backend.h) :
 * kal_freertos_region_leave() relâche les suspensions de l'ordonnanceur — elles sont forcément
 * celles de la tâche courante, aucune autre ne s'exécutant pendant la suspension — et rend leur
 * nombre ; kal_freertos_region_enter() les rétablit au réveil.
 */
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

UBaseType_t kal_freertos_region_leave(void){
   UBaseType_t n = 0;
   while(xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED) {
      (void)xTaskResumeAll();
      n++;
   }
   return n;
}

void kal_freertos_region_enter(UBaseType_t n){
   while(n--)
      vTaskSuspendAll();
}
