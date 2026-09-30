/*
 * Banc KAL (BANC-TEST-KAL-QEMU.md, T1-T8) — tests unitaires des macros de contexte du KAL
 * (kernel/core/kal.h, branche embOS) sous QEMU. Licence : voir LICENSE (MPL 1.1).
 *
 * Schéma de Lepton reproduit : le noyau ne manipule jamais un thread en cours d'exécution ; la
 * cible est bloquée (attente d'événement, comme l'attente du retour d'un appel système) pendant
 * que la tâche noyau agit sur son contexte. Ici :
 *   - tâche « contrôleur » (rôle de la tâche noyau), priorité 100 ;
 *   - tâche « cible », priorité 200 : elle s'exécute jusqu'à se bloquer, le contrôleur n'agit
 *     donc que sur une cible bloquée ; kal_regs_hold y maintient des motifs dans R4-R11.
 * Les séquences sont celles du noyau : core-segger/process.c (_sys_kill, _sys_kill_exit),
 * core-segger/fork.c (_sys_vfork, _sys_vfork_exit), core-segger/process.c (exec : contexte
 * de départ). Test choisi par l'argument semihosting ; code de sortie = nombre d'échecs.
 */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/errno.h"
#include "kernel/core/types.h"
#include "kernel/core/kernel_pthread.h"
#include "kernel/core/malloc.h"

#include "kal_test.h"

void kal_regs_hold(const uint32_t pattern[8], uint32_t out[8], void (*wait)(void));
void kal_regs_read(uint32_t out[8]);

#define EV_GO    0x01u   /* contrôleur → cible : reprise */
#define EV_H     0x02u   /* cible → contrôleur : gestionnaire (signal, fils vfork) exécuté */
#define EV_HX    0x04u   /* attente sans fin du gestionnaire (jamais signalé) */
#define EV_DONE  0x08u   /* cible → contrôleur : fin */

#define PRIO_CTRL    100
#define PRIO_TARGET  200
#define STACK_WORDS  1024

static OS_TASK ctrl_task;
static OS_TASK target_task;
static OS_STACKPTR uint32_t ctrl_stack[STACK_WORDS];
static OS_STACKPTR uint32_t target_stack[STACK_WORDS];
static kernel_pthread_t tp;

static const uint32_t pattern[8] = {
   0xA4A40004u, 0xA5A50005u, 0xA6A60006u, 0xA7A70007u,
   0xA8A80008u, 0xA9A90009u, 0xAAAA000Au, 0xABAB000Bu
};
static uint32_t regs_out[8];
static volatile int regs_ok;
static volatile int canary_ok;
static volatile int entry_count;
static volatile int self_ok, other_ok;
static volatile int handler_order[4];
static volatile int handler_count;
static volatile int handler_on_target;
static volatile int child_ran;
static volatile uint32_t exec_sp;
static uint32_t exec_regs[8];

/* --- outils ------------------------------------------------------------------------------ */
/* motif attendu au retour de kal_regs_hold (le contrôle négatif de T1 en attend un faux) */
static const uint32_t* volatile expected = pattern;
static const uint32_t wrong_pattern[8] = {
   0xA4A40004u, 0xA5A50005u, 0xA6A60006u, 0xA7A70007u,
   0xA8A80008u, 0xA9A90009u, 0xAAAA000Au, 0xABAB000Cu   /* dernier mot différent */
};

static int regs_match(void){
   return memcmp(regs_out, (const void*)expected, sizeof(pattern)) == 0;
}

static unsigned ctrl_wait(unsigned ev, unsigned timeout_ms){
   return (unsigned)OS_TASKEVENT_GetTimed((OS_TASKEVENT)ev, (OS_TIME)timeout_ms) & ev;
}

static int target_exists;
static uint32_t created_pstack;

static void target_create(void (*routine)(void), int with_start_context){
   /* une tâche embOS ne peut être recréée tant qu'elle existe (liste des tâches) */
   if(target_exists)
      OS_TASK_Terminate(&target_task);
   target_exists = 1;
   memset(&tp, 0, sizeof(tp));
   OS_TASK_EnterRegion();
   OS_TASK_Create(&target_task, "target", PRIO_TARGET, routine,
                  target_stack, sizeof(target_stack), 2);
   tp.tcb = &target_task;
   created_pstack = (uint32_t)target_task.pStack;
   if(with_start_context)
      __bckup_thread_start_context(tp.start_context, (&tp));
   OS_TASK_LeaveRegion();   /* la cible s'exécute jusqu'à se bloquer */
}

static void target_end(void){
   OS_TASKEVENT_Set(&ctrl_task, EV_DONE);
   for(;;)
      OS_TASKEVENT_GetBlocked(EV_HX);
}

/* --- routines de la cible ----------------------------------------------------------------- */
static void wait_go(void){
   while(!(OS_TASKEVENT_GetBlocked(EV_GO) & EV_GO)) {
   }
}

static void wait_timed(void){
   (void)OS_TASKEVENT_GetTimed(EV_GO, 50);
}

static void target_hold(void){
   kal_regs_hold(pattern, regs_out, wait_go);
   regs_ok = regs_match();
   target_end();
}

static void target_hold_timed(void){
   kal_regs_hold(pattern, regs_out, wait_timed);
   regs_ok = regs_match();
   target_end();
}

static void target_count_entry(void){
   entry_count++;
   target_end();
}

static void target_identity(void){
   self_ok  = __is_thread_self((&target_task)) ? 1 : 0;
   other_ok = __is_thread_self((&ctrl_task)) ? 0 : 1;
   target_end();
}

#define CANARY_WORDS 64
static void target_vfork_parent(void){
   volatile uint32_t canary[CANARY_WORDS];
   int i, ok = 1;
   for(i = 0; i < CANARY_WORDS; i++)
      canary[i] = 0xC0DE0000u + (uint32_t)i;
   kal_regs_hold(pattern, regs_out, wait_go);
   for(i = 0; i < CANARY_WORDS; i++)
      if(canary[i] != 0xC0DE0000u + (uint32_t)i)
         ok = 0;
   canary_ok = ok;
   regs_ok = regs_match();
   target_end();
}

static void exec_new_image(void){
   uint32_t sp;
   kal_regs_read(exec_regs);
   __asm volatile ("mov %0, sp" : "=r"(sp));
   exec_sp = sp;
   target_end();
}

/* gestionnaires exécutés par déroutement (__swap_signal_handler) : ils ne reviennent pas, la
   restauration vient du contrôleur (comme la sortie de gestionnaire par appel système) */
static void sig_handler_common(int id){
   handler_order[handler_count++] = id;
   handler_on_target = (OS_pCurrentTask == &target_task);
   OS_TASKEVENT_Set(&ctrl_task, EV_H);
   for(;;)
      OS_TASKEVENT_GetBlocked(EV_HX);
}
static void sig_handler1(void){ sig_handler_common(1); }
static void sig_handler2(void){ sig_handler_common(2); }

/* fils de vfork : même tâche, même pile ; écrase la pile vive du parent au-dessus de son SP */
static void vfork_child(void){
   uint32_t* p = (uint32_t*)((uint8_t*)tp.bckup_context.os_task.pStack + sizeof(OS_REGS_BASE));
   uint32_t* end = (uint32_t*)tp.start_context.os_task.pStack;
   child_ran = 1;
   while(p < end)
      *p++ = 0x5A5A5A5Au;
   OS_TASKEVENT_Set(&ctrl_task, EV_H);
   for(;;)
      OS_TASKEVENT_GetBlocked(EV_HX);
}

/* --- tests (contrôleur) -------------------------------------------------------------------- */
static void finish_target(const char* t){
   TEST_ASSERT(ctrl_wait(EV_DONE, 200), t);
}

/* T1 : sauvegarde / restitution de contexte ; contrôle négatif du comparateur.
   (Corrompre R4-R11 sans les restaurer n'est pas un contrôle valable : à l'instant du blocage ils
   appartiennent au code d'attente d'embOS, qui fait alors une faute au lieu de rendre un écart.) */
static int test_t1(void){
   OS_REGS_BASE* f;

   regs_ok = 0;
   target_create(target_hold, 0);
   __bckup_context(tp.bckup_context, (&tp));
   f = (OS_REGS_BASE*)target_task.pStack;
   f->OS_REG_R4 = f->OS_REG_R5 = f->OS_REG_R6 = f->OS_REG_R7 = 0xDEAD0000u;
   f->OS_REG_R8 = f->OS_REG_R9 = f->OS_REG_R10 = f->OS_REG_R11 = 0xDEAD0001u;
   __rstr_context(tp.bckup_context, (&tp));
   OS_TASKEVENT_Set(&target_task, EV_GO);
   finish_target("T1 : fin de la cible");
   TEST_ASSERT(regs_ok, "T1 : R4-R11 restaurés après corruption du contexte sauvegardé");

   /* contrôle négatif : un motif différent d'un seul mot doit être détecté */
   regs_ok = 1;
   expected = wrong_pattern;
   target_create(target_hold, 0);
   OS_TASKEVENT_Set(&target_task, EV_GO);
   finish_target("T1 (négatif) : fin de la cible");
   expected = pattern;
   TEST_ASSERT(!regs_ok, "T1 (négatif) : écart de motif non détecté — harnais inopérant");
   return 0;
}

/* T2 : contexte de démarrage sauvegardé à la création (décision 2026-09-30 : aligné sur Lepton,
   qui ne redémarre jamais un thread depuis ce contexte ; il sert de référence de pile au vfork,
   T7). Sous embOS 5.20 le PC de départ est le trampoline OS_StartTask (symbole exporté par la
   bibliothèque), la routine étant rangée sur la pile au-dessus du cadre OS_REGS_BASE. */
extern void OS_StartTask(void);

static int test_t2(void){
   OS_REGS_BASE* f;
   uint32_t sp;

   entry_count = 0;
   target_create(target_count_entry, 1);
   f = &tp.start_context.os_regs;
   sp = (uint32_t)tp.start_context.os_task.pStack;
   kal_test_put_u32("T2 : PC de départ = ", f->OS_REG_PC);
   TEST_ASSERT(f->OS_REG_XPSR & 0x01000000u, "T2 : bit T d'xPSR du contexte de départ");
   TEST_ASSERT(sp >= (uint32_t)target_stack && sp < (uint32_t)target_stack + sizeof(target_stack),
               "T2 : pile de départ dans la pile de la tâche");
   TEST_ASSERT(sp == created_pstack, "T2 : pile de départ = pStack du TCB à la création");
   TEST_ASSERT((f->OS_REG_PC & ~1u) == ((uint32_t)OS_StartTask & ~1u),
               "T2 : PC de départ = trampoline embOS OS_StartTask");
   finish_target("T2 : premier démarrage");
   TEST_ASSERT(entry_count == 1, "T2 : point d'entrée exécuté une fois au démarrage");
   return 0;
}

/* T3 : identité */
static int test_t3(void){
   self_ok = other_ok = 0;
   target_create(target_identity, 0);
   finish_target("T3 : fin de la cible");
   TEST_ASSERT(self_ok, "T3 : __is_thread_self vrai pour la tâche courante");
   TEST_ASSERT(other_ok, "T3 : __is_thread_self faux pour une autre tâche");
   TEST_ASSERT(!__is_thread_self((&target_task)), "T3 : faux côté contrôleur pour la cible");
   return 0;
}

/* déroutement simple (_sys_kill) puis sortie (_sys_kill_exit) */
static int deroute(void (*handler)(void)){
   __bckup_context(tp.bckup_context, (&tp));
   __swap_signal_handler((&tp), handler);
   tp.stat |= PTHREAD_STATUS_SIGHANDLER;
   return ctrl_wait(EV_H, 200) ? 0 : -1;
}

static void deroute_exit(void){
   __rstr_context(tp.bckup_context, (&tp));
   tp.stat &= ~PTHREAD_STATUS_SIGHANDLER;
   __set_active_pthread((&tp));
}

/* T4 : déroutement simple */
static int test_t4(void){
   regs_ok = 0;
   handler_count = 0;
   handler_on_target = 0;
   target_create(target_hold, 0);
   TEST_ASSERT(deroute(sig_handler1) == 0, "T4 : gestionnaire exécuté");
   TEST_ASSERT(handler_on_target, "T4 : gestionnaire exécuté dans la tâche cible");
   deroute_exit();
   OS_TASKEVENT_Set(&target_task, EV_GO);
   finish_target("T4 : reprise au point d'interruption");
   TEST_ASSERT(handler_count == 1, "T4 : un seul passage dans le gestionnaire");
   TEST_ASSERT(regs_ok, "T4 : R4-R11 intacts après retour du gestionnaire");
   return 0;
}

/* T5 : déroutement pendant une attente temporisée (appel système préemptible) */
static int test_t5(void){
   regs_ok = 0;
   handler_count = 0;
   target_create(target_hold_timed, 0);
   TEST_ASSERT(deroute(sig_handler1) == 0, "T5 : gestionnaire exécuté pendant l'attente");
   deroute_exit();
   /* pas d'EV_GO : l'attente temporisée (50 ms) doit se terminer d'elle-même */
   finish_target("T5 : l'attente interrompue se termine");
   TEST_ASSERT(regs_ok, "T5 : R4-R11 intacts après l'attente interrompue");
   return 0;
}

/* T6 : second signal pendant le gestionnaire : mis en attente (PTHREAD_STATUS_SIGHANDLER,
   _sys_kill), puis délivré à la sortie du premier (_sys_kill_exit) */
static int test_t6(void){
   int pending = 0;
   regs_ok = 0;
   handler_count = 0;
   target_create(target_hold, 0);
   TEST_ASSERT(deroute(sig_handler1) == 0, "T6 : premier gestionnaire");
   if(tp.stat & PTHREAD_STATUS_SIGHANDLER)
      pending = 1;                           /* second signal : en attente */
   else
      TEST_ASSERT(0, "T6 : statut SIGHANDLER absent pendant le gestionnaire");
   __rstr_context(tp.bckup_context, (&tp));  /* sortie du premier */
   tp.stat &= ~PTHREAD_STATUS_SIGHANDLER;
   if(pending)
      TEST_ASSERT(deroute(sig_handler2) == 0, "T6 : second gestionnaire délivré à la sortie du premier");
   deroute_exit();
   OS_TASKEVENT_Set(&target_task, EV_GO);
   finish_target("T6 : reprise après les deux gestionnaires");
   TEST_ASSERT(handler_count == 2 && handler_order[0] == 1 && handler_order[1] == 2,
               "T6 : gestionnaires exécutés dans l'ordre 1, 2, sans imbrication");
   TEST_ASSERT(regs_ok, "T6 : R4-R11 intacts après les deux gestionnaires");
   return 0;
}

/* T7 : vfork (_sys_vfork / _sys_vfork_exit) */
static int test_t7(void){
   kernel_pthread_t* backup;
   regs_ok = canary_ok = child_ran = 0;
   target_create(target_vfork_parent, 1);
   backup = (kernel_pthread_t*)_sys_malloc(sizeof(kernel_pthread_t));
   if(!backup) {
      TEST_ASSERT(0, "T7 : allocation");
      return -1;
   }
   memcpy(backup, &tp, sizeof(kernel_pthread_t));
   __bckup_context(backup->bckup_context, backup);
   __bckup_stack(backup);
   tp.bckup_context = backup->bckup_context;        /* adresses vues par le fils */
   /* le fils poursuit sur la même tâche et la même pile */
   __swap_signal_handler((&tp), vfork_child);
   TEST_ASSERT(ctrl_wait(EV_H, 200), "T7 : fils exécuté");
   /* sortie du fils : restitution du parent */
   memcpy(&tp, backup, sizeof(kernel_pthread_t));
   __rstr_stack((&tp));
   __rstr_context(tp.bckup_context, (&tp));
   __set_active_pthread((&tp));
   _sys_free(backup);
   OS_TASKEVENT_Set(&target_task, EV_GO);
   finish_target("T7 : reprise du parent");
   TEST_ASSERT(child_ran, "T7 : fils exécuté sur la tâche du parent");
   TEST_ASSERT(canary_ok, "T7 : pile du parent restituée (canari intact)");
   TEST_ASSERT(regs_ok, "T7 : R4-R11 du parent restitués");
   return 0;
}

/* T8 : exec (recouvrement), comme _sys_exec (décision 2026-09-30) : la tâche de l'ancienne image
   est terminée, une nouvelle tâche est créée sur le même pthread et la même pile avec le nouveau
   point d'entrée, et son contexte de départ est sauvegardé (référence du vfork). */
static int test_t8(void){
   uint32_t top = (uint32_t)target_stack + sizeof(target_stack);
   exec_sp = 0;
   regs_ok = 0;
   target_create(target_hold, 0);            /* ancienne image, bloquée avec ses motifs */
   target_create(exec_new_image, 1);         /* exec : terminaison, recréation, contexte de départ */
   finish_target("T8 : nouvelle image démarrée");
   kal_test_put_u32("T8 : SP de la nouvelle image = ", exec_sp);
   TEST_ASSERT(exec_sp > top - 256u && exec_sp <= top, "T8 : pile réinitialisée (haut de pile)");
   TEST_ASSERT(memcmp(exec_regs, pattern, sizeof(pattern)) != 0,
               "T8 : aucun registre de l'ancien flux ne survit");
   OS_TASKEVENT_Set(&target_task, EV_GO);    /* l'ancienne image ne doit pas reprendre */
   OS_TASK_Delay(20);
   TEST_ASSERT(!regs_ok, "T8 : l'ancienne image ne reprend pas");
   return 0;
}

static int test_harness_fail(void){
   TEST_ASSERT(0, "échec volontaire (le harnais doit rendre un code non nul)");
   return 0;
}

/* --- aiguillage ---------------------------------------------------------------------------- */
static const struct { const char* name; int (*fn)(void); } tests[] = {
   { "T1", test_t1 }, { "T2", test_t2 }, { "T3", test_t3 }, { "T4", test_t4 },
   { "T5", test_t5 }, { "T6", test_t6 }, { "T7", test_t7 }, { "T8", test_t8 },
   { "HARNESS_FAIL", test_harness_fail },
};

static char cmdline[128];

static const char* selected_test(void){
   const char* last = cmdline;
   const char* p;
   for(p = cmdline; *p; p++)
      if(*p == ' ' && p[1])
         last = p + 1;
   return last;
}

static void controller(void){
   const char* name = selected_test();
   unsigned i;
   for(i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
      if(!strcmp(tests[i].name, name)) {
         kal_test_puts("kal_bench : ");
         kal_test_puts(name);
         kal_test_puts("\n");
         tests[i].fn();
         kal_test_puts(kal_test_failures ? "kal_bench : ÉCHEC\n" : "kal_bench : OK\n");
         kal_test_exit(kal_test_failures);
      }
   }
   kal_test_puts("kal_bench : test inconnu\n");
   kal_test_exit(125);
}

int main(void){
   if(kal_test_cmdline(cmdline, sizeof(cmdline)) < 0)
      cmdline[0] = 0;
   OS_IncDI();
   OS_Init();
   OS_InitHW();
   OS_TASK_Create(&ctrl_task, "controller", PRIO_CTRL, controller,
                  ctrl_stack, sizeof(ctrl_stack), 2);
   OS_Start();
   return 0;
}
