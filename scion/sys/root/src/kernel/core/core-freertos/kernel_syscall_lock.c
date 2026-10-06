/*
 * Lepton — verrou des appels système, backend FreeRTOS. Licence : voir LICENSE (MPL 1.1).
 * Copie de core-segger/kernel_syscall_lock.c (étape 7) : indépendant du micro-noyau (kernel_sem) ;
 * sous FreeRTOS aussi, un mutex ne peut être rendu que par son propriétaire (configASSERT).
 *
 * __mk_syscall prend le verrou dans le thread appelant, la tâche noyau le rend en fin de
 * traitement (_kernel_syscall). Un mutex embOS ne peut être rendu que par son propriétaire
 * (embOS 5.20 : OS_ERR_MUTEX_OWNER en mode DP, état incohérent sinon) : le verrou est un
 * sémaphore de valeur initiale 1 (décision 2026-09-30, étape 3), le propriétaire est tenu par
 * Lepton (kernel_syscall_lock_owner, informatif : kernel_pthread.c).
 * Comme l'ancien kernel_mutex, sans effet tant que le noyau est en mode statique (amorçage).
 */
#include <stdint.h>
#include <stdarg.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/errno.h"
#include "kernel/core/types.h"
#include "kernel/core/kernel.h"
#include "kernel/core/kernel_pthread.h"
#include "kernel/core/kernel_sem.h"

static kernel_sem_t kernel_syscall_sem;
kernel_pthread_t* volatile kernel_syscall_lock_owner = (kernel_pthread_t*)0;

int kernel_syscall_lock_init(void){
   kernel_syscall_lock_owner = (kernel_pthread_t*)0;
   return kernel_sem_init(&kernel_syscall_sem, 0, 1);
}

int kernel_syscall_lock(void){
   if(__kernel_is_in_static_mode())
      return 0;
   kernel_sem_wait(&kernel_syscall_sem);
   kernel_syscall_lock_owner = kernel_pthread_self();
   return 0;
}

int kernel_syscall_trylock(void){
   if(__kernel_is_in_static_mode())
      return 0;
   if(kernel_sem_trywait(&kernel_syscall_sem) < 0)
      return -EBUSY;
   kernel_syscall_lock_owner = kernel_pthread_self();
   return 0;
}

int kernel_syscall_unlock(void){
   if(__kernel_is_in_static_mode())
      return 0;
   kernel_syscall_lock_owner = (kernel_pthread_t*)0;
   return kernel_sem_post(&kernel_syscall_sem);
}
