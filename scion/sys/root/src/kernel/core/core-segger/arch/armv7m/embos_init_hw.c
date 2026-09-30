/*
 * Lepton — intégration matérielle d'embOS sur ARMv7-M (SysTick). Licence : voir LICENSE (MPL 1.1).
 *
 * Écrit pour Lepton d'après l'API documentée d'embOS V5 (UM01001, UM01039) ; aucun fichier
 * d'exemple Segger (décision 2026-09-30). Fonctions attendues par la bibliothèque :
 *   OS_InitHW()       : SysTick à la fréquence de tick, priorités, OS_TIME_ConfigSysTimer ;
 *   SysTick_Handler() : OS_TICK_Handle() entre OS_INT_EnterNestable / OS_INT_LeaveNestable ;
 *   OS_Idle()         : boucle d'attente (pas de WFI : QEMU et débogueur) ;
 *   OS_Error()        : appelée par les bibliothèques de débogage (DP) ; mémorise le code et
 *                       s'arrête (point d'arrêt débogueur).
 * Registres SysTick et SCB architecturaux (ARMv7-M) ; fréquence : SystemCoreClock (carte).
 */
#include <stdint.h>
#include "RTOS.h"

extern uint32_t SystemCoreClock;

#define LEPTON_EMBOS_TICK_FREQ   1000u   /* 1 ms */

/* SysTick (ARMv7-M ARM, B3.3) */
#define SYST_CSR   (*(volatile uint32_t*)0xE000E010u)
#define SYST_RVR   (*(volatile uint32_t*)0xE000E014u)
#define SYST_CVR   (*(volatile uint32_t*)0xE000E018u)
#define SYST_CSR_ENABLE    (1u << 0)
#define SYST_CSR_TICKINT   (1u << 1)
#define SYST_CSR_CLKSOURCE (1u << 2)
/* SCB : ICSR (PENDSTSET), SHPR3 (priorités PendSV, SysTick) */
#define SCB_ICSR   (*(volatile uint32_t*)0xE000ED04u)
#define SCB_ICSR_PENDSTSET (1u << 26)
#define SCB_SHPR3  (*(volatile uint32_t*)0xE000ED20u)

static unsigned int _lepton_get_systick_cycles(void){
   return (unsigned int)SYST_CVR;
}

static unsigned int _lepton_get_systick_pending(void){
   return (SCB_ICSR & SCB_ICSR_PENDSTSET) ? 1u : 0u;
}

void SysTick_Handler(void){
   OS_INT_EnterNestable();
   OS_TICK_Handle();
   OS_INT_LeaveNestable();
}

void OS_InitHW(void){
   OS_SYSTIMER_CONFIG sys_timer = {
      SystemCoreClock,               /* fréquence du compteur */
      LEPTON_EMBOS_TICK_FREQ,        /* fréquence d'interruption */
      OS_TIMER_DOWNCOUNTING,
      _lepton_get_systick_cycles,
      _lepton_get_systick_pending
   };

   OS_INT_IncDI();
   /* SysTick au-dessus de PendSV, qu'embOS place à la priorité la plus basse, et dans la plage
      masquée par embOS (>= 0x80) : 0xC0 convient à 3 bits (QEMU) comme à 4 bits (STM32F4)
      de priorité implémentés (bits 31:24 de SHPR3). */
   SCB_SHPR3 = (SCB_SHPR3 & 0x00FFFFFFu) | (0xC0u << 24);
   SYST_RVR = (SystemCoreClock / LEPTON_EMBOS_TICK_FREQ) - 1u;
   SYST_CVR = 0;
   SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;
   OS_TIME_ConfigSysTimer(&sys_timer);
   OS_INT_DecRI();
}

void OS_Idle(void){
   for(;;) {
   }
}

/* Taille du tampon de communication embOSView par mémoire J-Link (RTOS.h, fournie par
 * l'application) ; embOSView n'est pas utilisé par Lepton. */
const OS_U32 OS_JLINKMEM_BufferSize = 32u;

volatile OS_STATUS lepton_embos_last_error;

void OS_Error(OS_STATUS ErrCode){
   OS_TASK_EnterRegion();
   OS_INT_Disable();
   lepton_embos_last_error = ErrCode;
   for(;;) {
   }
}
