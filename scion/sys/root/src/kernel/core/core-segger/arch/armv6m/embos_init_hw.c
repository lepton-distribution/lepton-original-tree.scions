/*
 * Lepton — intégration matérielle d'embOS sur ARMv6-M (SysTick), étape 6. Licence : voir LICENSE (MPL 1.1).
 *
 * Écrit pour Lepton d'après l'API documentée d'embOS V5 (UM01001, UM01039) ; aucun fichier
 * d'exemple Segger (décision 2026-09-30). Fonctions attendues par la bibliothèque :
 *   OS_InitHW()       : SysTick à la fréquence de tick, priorités, OS_TIME_ConfigSysTimer ;
 *   SysTick_Handler() : OS_TICK_Handle() entre OS_INT_EnterNestable / OS_INT_LeaveNestable ;
 *   OS_Idle()         : boucle d'attente (pas de WFI : QEMU et débogueur) ;
 *   OS_Error()        : appelée par les bibliothèques de débogage (DP) ; mémorise le code et
 *                       s'arrête (point d'arrêt débogueur).
 * Registres SysTick et SCB architecturaux (mêmes adresses qu'en ARMv7-M ; SHPR3 accessible par
 * mot seulement) ; fréquence : SystemCoreClock (carte). Copie de arch/armv7m/embos_init_hw.c,
 * seule la priorité de SysTick diffère.
 */
#include <stdint.h>
#include "RTOS.h"

extern uint32_t SystemCoreClock;

/* fréquence du tick = _SC_CLK_TCK du noyau (timer.h) : __KERNEL_CLK_TCK, cmake/kal/embos.cmake */
#define LEPTON_EMBOS_TICK_FREQ   ((unsigned int)__KERNEL_CLK_TCK)

/* SysTick (ARMv6-M ARM, B3.3) */
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
   /* SysTick juste au-dessus de PendSV, qu'embOS place à la priorité la plus basse : ARMv6-M
      implémente 2 bits de priorité (bits 7:6), 0x80 = niveau 2 sur 0-3 (exemple embOS SAMD20 :
      (1 << __NVIC_PRIO_BITS) - 2). Pas de BASEPRI : embOS masque toutes les interruptions
      (PRIMASK) dans ses sections critiques. */
   SCB_SHPR3 = (SCB_SHPR3 & 0x00FFFFFFu) | (0x80u << 24);
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
