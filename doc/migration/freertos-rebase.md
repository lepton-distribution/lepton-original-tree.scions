# Remise à niveau de core-freertos (freertos_rebase.py)

Base : core-segger@037cd59. Classes : freertos et entete gardés, ancien abandonné.

| Fichier | Bloc | Lignes base | Classe | Raison | Extrait |
|---|---|---|---|---|---|
| core_rttimer.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| core_rttimer.c | 1 | 31-30 | ancien | lignée | `#include "kernel/core/kernelconf.h"` |
| core_rttimer.c | 2 | 54-55 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| core_rttimer.c | 3 | 71-72 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| core_rttimer.c | 4 | 88-89 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| core_rttimer.c | 5 | 105-106 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| core_rttimer.c | 6 | 122-123 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| core_rttimer.c | — | — | **fusion** | 0 conflit(s) | |
| fork.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| fork.c | 1 | 31-30 | ancien | lignée | `#include <stdarg.h>` |
| fork.c | 2 | 32-31 | ancien | lignée | `#include "kernel/core/kernelconf.h"` |
| fork.c | 3 | 59-67 | ancien | lignée | `#if defined(WIN32) && defined(LEPTON_CHKESP)` |
| fork.c | — | — | **fusion** | 0 conflit(s) | |
| heap.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| heap.c | — | — | **fusion** | 0 conflit(s) | |
| kernel.c | 0 | 35-34 | ancien | lignée | `#include <string.h>` |
| kernel.c | 1 | 85-85 | ancien | lignée | `` |
| kernel.c | 2 | 87-86 | freertos | forcé | `volatile int kernel_in_interrupt=0;` |
| kernel.c | 3 | 93-110 | ancien | lignée | `#if (__tauon_cpu_core__ == __tauon_cpu_core_arm_cortexM0__)` |
| kernel.c | 4 | 113-115 | freertos | mot-clé | `#define KERNEL_PRIORITY    (configMAX_PRIORITIES-1)` |
| kernel.c | 5 | 118-117 | freertos | forcé | `volatile int kernel_in_interrupt;` |
| kernel.c | 6 | 205-205 | ancien | lignée | `__add_syscall(_syscall_pthread_join),` |
| kernel.c | 7 | 257-267 | ancien | lignée | `//chack syscall nb validity` |
| kernel.c | 8 | 614-616 | ancien | lignée | `}else if(pdev_lst[dev]->dev_name[0]=='i'` |
| kernel.c | 9 | 621-623 | ancien | lignée | `}else if(pdev_lst[dev]->dev_name[0]=='s'` |
| kernel.c | 10 | 667-667 | ancien | lignée | `#ifdef __KERNEL_DEV_TTY` |
| kernel.c | 11 | 670-671 | ancien | lignée | `desc_t desc_tty;` |
| kernel.c | 12 | 699-699 | ancien | lignée | `\| Name:        _kernel_warmup_stream` |
| kernel.c | 13 | 706-755 | ancien | lignée | `void _kernel_warmup_stream(void){` |
| kernel.c | 14 | 757-763 | ancien | lignée | `if((desc_2 = _vfs_open("/dev/hd/hdd",O_RDWR,0))<0)` |
| kernel.c | 15 | 765-769 | ancien | lignée | `if(_vfs_ioctl(desc_1,I_LINK,desc_2)<0)` |
| kernel.c | 16 | 797-801 | ancien | lignée | `#if defined(__KERNEL_RTC_DEV_NAME__) && defined(__KERNEL_DEV` |
| kernel.c | 17 | 804-804 | ancien | lignée | `if((desc = _vfs_open("/dev/rtc0",O_RDONLY,0))<0) //ST m41t81` |
| kernel.c | 18 | 806-807 | ancien | lignée | `` |
| kernel.c | 19 | 845-845 | ancien | lignée | `if((desc = _vfs_open("/dev/rtt0",O_RDONLY,0))<0) //ST m41t81` |
| kernel.c | 20 | 847-848 | ancien | lignée | `}` |
| kernel.c | 21 | 860-861 | ancien | lignée | `return 0;` |
| kernel.c | 22 | 1190-1193 | ancien | lignée | `//   rttmr_attr.tm_msec=__KERNEL_ALARM_TIMER;` |
| kernel.c | 23 | 1216-1216 | ancien | lignée | `#if defined (__KERNEL_WARMUP_I2C) && (__KERNEL_WARMUP_I2C==1` |
| kernel.c | 24 | 1218-1218 | ancien | lignée | `#endif` |
| kernel.c | 25 | 1225-1224 | ancien | lignée | `//` |
| kernel.c | — | — | **fusion** | 0 conflit(s) | |
| kernel_clock.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| kernel_clock.c | 1 | 31-30 | ancien | lignée | `#include "kernel/core/kernelconf.h"` |
| kernel_clock.c | — | — | **fusion** | 0 conflit(s) | |
| kernel_elfloader.c | 0 | 34-33 | ancien | lignée | `#include <stdarg.h>` |
| kernel_elfloader.c | 1 | 35-35 | ancien | lignée | `#include <string.h>` |
| kernel_elfloader.c | 2 | 43-42 | ancien | lignée | `#include "kernel/core/dirent.h"` |
| kernel_elfloader.c | 3 | 50-49 | ancien | lignée | `#include "kernel/fs/vfs/vfskernel.h"` |
| kernel_elfloader.c | 4 | 164-164 | ancien | lignée | `{0, 0, (unsigned char*) -1, 0, (unsigned char*) -1, -1,` |
| kernel_elfloader.c | 5 | 176-176 | ancien | lignée | `string->bufend = (unsigned char*)buf+sizeof(buf);` |
| kernel_elfloader.c | 6 | 658-665 | ancien | lignée | `#if (__tauon_compiler__==__compiler_iar_arm__)` |
| kernel_elfloader.c | — | — | **fusion** | 0 conflit(s) | |
| kernel_object.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| kernel_object.c | 1 | 29-28 | ancien | lignée | `#include <stdarg.h>` |
| kernel_object.c | 2 | 30-30 | ancien | lignée | `#include <string.h>` |
| kernel_object.c | 3 | 32-31 | ancien | lignée | `#include "kernel/core/kernelconf.h"` |
| kernel_object.c | 4 | 50-54 | ancien | lignée | `static kernel_object_t*  kernel_object_pool_head = (kernel_o` |
| kernel_object.c | 5 | 145-145 | ancien | lignée | `#ifndef CPU_M16C62` |
| kernel_object.c | 6 | 149-149 | ancien | lignée | `#endif` |
| kernel_object.c | 7 | 164-165 | ancien | lignée | `char* name;` |
| kernel_object.c | 8 | 167-168 | ancien | lignée | `name = va_arg( ap, char*);` |
| kernel_object.c | 9 | 171-171 | ancien | lignée | `p->object.kernel_object_sem.name=name;` |
| kernel_object.c | 10 | 173-173 | ancien | lignée | `p->object.kernel_object_sem.ref_count=0;` |
| kernel_object.c | 11 | 539-573 | ancien | lignée | `/*--------------------------------------------` |
| kernel_object.c | — | — | **fusion** | 0 conflit(s) | |
| kernel_pthread.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| kernel_pthread.c | 1 | 59-58 | ancien | lignée | `#include <string.h>` |
| kernel_pthread.c | 2 | 60-59 | ancien | lignée | `#include "kernel/core/kernelconf.h"` |
| kernel_pthread.c | 3 | 63-64 | ancien | lignée | `#include "kernel/core/kernel_pthread_tsd.h"` |
| kernel_pthread.c | 4 | 67-68 | ancien | lignée | `#include "kernel/fs/vfs/vfstypes.h"` |
| kernel_pthread.c | 5 | 75-74 | freertos | mot-clé | `/*lint !e971 Unqualified char types are allowed for strings ` |
| kernel_pthread.c | 6 | 84-84 | ancien | lignée | `\| Name:        new_thread` |
| kernel_pthread.c | 7 | 122-122 | ancien | lignée | `\| Name:        kernel_insert_gpthread` |
| kernel_pthread.c | 8 | 144-144 | ancien | lignée | `\| Name:        kernel_remove_gpthread` |
| kernel_pthread.c | 9 | 166-166 | ancien | lignée | `\| Name:get_pthread_id` |
| kernel_pthread.c | 10 | 185-185 | ancien | lignée | `\| Name:put_pthread_id` |
| kernel_pthread.c | 11 | 198-198 | ancien | lignée | `\| Name:        kernel_pthread_alloca` |
| kernel_pthread.c | 12 | 230-272 | ancien | lignée | `\| Name:pthread_routine` |
| kernel_pthread.c | 13 | 280-279 | ancien | lignée | `` |
| kernel_pthread.c | 14 | 281-280 | ancien | lignée | `pthread_exit_t pthread_exit_dt;` |
| kernel_pthread.c | 15 | 283-285 | ancien | lignée | `pthread->exit=pthread->start_routine(pthread->arg);` |
| kernel_pthread.c | 16 | 287-293 | ancien | lignée | `//call kernel. signal thread termination` |
| kernel_pthread.c | 17 | 298-298 | ancien | lignée | `\| Name:pthread_create` |
| kernel_pthread.c | 18 | 336-338 | ancien | lignée | `//alloc tcb` |
| kernel_pthread.c | 19 | 342-343 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_pthread.c | 20 | 347-346 | freertos | mot-clé | `xTaskHandle freertos_task_handle;` |
| kernel_pthread.c | 21 | 352-351 | ancien | lignée | `` |
| kernel_pthread.c | 22 | 353-362 | freertos | mot-clé | `#if (configSUPPORT_STATIC_ALLOCATION==1)` |
| kernel_pthread.c | 23 | 366-365 | ancien | lignée | `static const uint8_t ucExpectedStackBytes[] = {` |
| kernel_pthread.c | 24 | 367-368 | ancien | lignée | `uint8_t _align = (4-(_stack_addr%4))+4+sizeof(ucExpectedStac` |
| kernel_pthread.c | 25 | 378-379 | ancien | lignée | `kernel_sem_init(&thread->io_sem,0,0);` |
| kernel_pthread.c | 26 | 386-386 | ancien | lignée | `\| Name:pthread_kill` |
| kernel_pthread.c | 27 | 393-393 | ancien | lignée | `int   kernel_pthread_kill(kernel_pthread_t* thread, int sig)` |
| kernel_pthread.c | 28 | 404-404 | ancien | lignée | `\| Name:pthread_cancel` |
| kernel_pthread.c | 29 | 411-411 | ancien | lignée | `int   kernel_pthread_cancel(kernel_pthread_t* thread){` |
| kernel_pthread.c | 30 | 440-440 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_pthread.c | 31 | 442-443 | freertos | mot-clé | `xTaskHandle whois_lock_kernel_mutex = xSemaphoreGetMutexHold` |
| kernel_pthread.c | 32 | 445-454 | freertos | remplace embOS | `//free kernel mutex. it was taken by this pthread.` |
| kernel_pthread.c | 33 | 456-456 | freertos | mot-clé | `//destroy event group` |
| kernel_pthread.c | 34 | 458-468 | freertos | remplace embOS | `//` |
| kernel_pthread.c | 35 | 473-474 | ancien | lignée | `` |
| kernel_pthread.c | 36 | 489-489 | ancien | lignée | `\| Name:pthread_self` |
| kernel_pthread.c | 37 | 512-523 | ancien | lignée | `/*-------------------------------------------` |
| kernel_pthread.c | — | — | **fusion** | 1 conflit(s) | |
| kernel_pthread_mutex.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| kernel_pthread_mutex.c | 1 | 25-25 | entete | licence | `` |
| kernel_pthread_mutex.c | 2 | 30-29 | ancien | lignée | `#include <stdarg.h>` |
| kernel_pthread_mutex.c | 3 | 31-30 | ancien | lignée | `#include "kernel/core/kernelconf.h"` |
| kernel_pthread_mutex.c | 4 | 32-33 | ancien | lignée | `#include "kernel/core/interrupt.h"` |
| kernel_pthread_mutex.c | 5 | 35-36 | ancien | lignée | `#include "kernel/core/kernel_pthread_mutex.h"` |
| kernel_pthread_mutex.c | 6 | 57-58 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_pthread_mutex.c | 7 | 73-73 | ancien | lignée | `//` |
| kernel_pthread_mutex.c | 8 | 77-99 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_pthread_mutex.c | 9 | 116-144 | freertos | remplace embOS | `//not used` |
| kernel_pthread_mutex.c | 10 | 157-157 | ancien | lignée | `//` |
| kernel_pthread_mutex.c | 11 | 161-162 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_pthread_mutex.c | 12 | 177-177 | ancien | lignée | `//` |
| kernel_pthread_mutex.c | 13 | 181-182 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_pthread_mutex.c | 14 | 198-198 | ancien | lignée | `//` |
| kernel_pthread_mutex.c | 15 | 202-203 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_pthread_mutex.c | — | — | **fusion** | 1 conflit(s) | |
| kernel_sem.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| kernel_sem.c | 1 | 29-28 | ancien | lignée | `#include <stdarg.h>` |
| kernel_sem.c | 2 | 30-29 | ancien | lignée | `#include "kernel/core/kernelconf.h"` |
| kernel_sem.c | 3 | 31-30 | ancien | lignée | `#include "kernel/core/interrupt.h"` |
| kernel_sem.c | 4 | 58-59 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_sem.c | 5 | 75-76 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_sem.c | 6 | 92-93 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_sem.c | 7 | 106-105 | ancien | lignée | `` |
| kernel_sem.c | 8 | 109-110 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_sem.c | 9 | 132-133 | freertos | mot-clé | `//` |
| kernel_sem.c | 10 | 138-138 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_sem.c | 11 | 140-140 | freertos | mot-clé | `if(!xSemaphoreTake(kernel_sem->sem, (portTickType)(timeout/p` |
| kernel_sem.c | 12 | 144-144 | freertos | mot-clé | `if(!xSemaphoreTake(kernel_sem->sem, (portTickType)(0)))   //` |
| kernel_sem.c | 13 | 147-147 | freertos | mot-clé | `while(!xSemaphoreTake(kernel_sem->sem, portMAX_DELAY));` |
| kernel_sem.c | 14 | 155-155 | ancien | lignée | `\| Name:        kernel_sem_trywait` |
| kernel_sem.c | 15 | 165-166 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_sem.c | 16 | 183-184 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_sem.c | — | — | **fusion** | 0 conflit(s) | |
| kernel_sigqueue.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| kernel_sigqueue.c | 1 | 30-29 | ancien | lignée | `#include <stdarg.h>` |
| kernel_sigqueue.c | 2 | 38-37 | ancien | lignée | `#ifdef __KERNEL_POSIX_REALTIME_SIGNALS` |
| kernel_sigqueue.c | 3 | 67-67 | ancien | lignée | `if((p->kernel_sem = kernel_object_manager_get(pp_kernel_obje` |
| kernel_sigqueue.c | 4 | 114-115 | ancien | lignée | `kernel_pthread_mutex_lock(&p->kernel_mutex->object.kernel_ob` |
| kernel_sigqueue.c | 5 | 123-124 | ancien | lignée | `kernel_pthread_mutex_unlock(&p->kernel_mutex->object.kernel_` |
| kernel_sigqueue.c | 6 | 132-133 | ancien | lignée | `kernel_pthread_mutex_unlock(&p->kernel_mutex->object.kernel_` |
| kernel_sigqueue.c | 7 | 138-139 | ancien | lignée | `kernel_pthread_mutex_unlock(&p->kernel_mutex->object.kernel_` |
| kernel_sigqueue.c | 8 | 162-163 | ancien | lignée | `kernel_pthread_mutex_lock(&p->kernel_mutex->object.kernel_ob` |
| kernel_sigqueue.c | 9 | 179-180 | ancien | lignée | `kernel_pthread_mutex_unlock(&p->kernel_mutex->object.kernel_` |
| kernel_sigqueue.c | 10 | 189-190 | ancien | lignée | `kernel_pthread_mutex_unlock(&p->kernel_mutex->object.kernel_` |
| kernel_sigqueue.c | 11 | 214-215 | ancien | lignée | `kernel_pthread_mutex_lock(&p->kernel_mutex->object.kernel_ob` |
| kernel_sigqueue.c | 12 | 225-226 | ancien | lignée | `kernel_pthread_mutex_unlock(&p->kernel_mutex->object.kernel_` |
| kernel_sigqueue.c | 13 | 235-236 | ancien | lignée | `kernel_pthread_mutex_unlock(&p->kernel_mutex->object.kernel_` |
| kernel_sigqueue.c | 14 | 317-317 | ancien | lignée | `#endif` |
| kernel_sigqueue.c | — | — | **fusion** | 0 conflit(s) | |
| kernel_timer.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| kernel_timer.c | 1 | 30-29 | ancien | lignée | `#include <stdarg.h>` |
| kernel_timer.c | 2 | 32-35 | ancien | lignée | `#include "kernel/core/syscall.h"` |
| kernel_timer.c | 3 | 75-75 | freertos | mot-clé | `void kernel_timer_generic_callback(xTimerHandle pxTimer ){` |
| kernel_timer.c | 4 | 80-81 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_timer.c | 5 | 85-85 | ancien | lignée | `if(p_kernel_timer->interval && p_kernel_timer->itimerspec.it` |
| kernel_timer.c | 6 | 87-89 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_timer.c | 7 | 91-91 | ancien | lignée | `}else if(!p_kernel_timer->interval && p_kernel_timer->itimer` |
| kernel_timer.c | 8 | 93-95 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_timer.c | 9 | 97-97 | ancien | lignée | `//don't send signal` |
| kernel_timer.c | 10 | 175-176 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_timer.c | 11 | 191-191 | ancien | lignée | `int elapse_time_ms=0;` |
| kernel_timer.c | 12 | 199-202 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_timer.c | 13 | 238-238 | freertos | mot-clé | `#ifdef __KERNEL_UCORE_FREERTOS` |
| kernel_timer.c | 14 | 240-240 | freertos | remplace embOS | `int elsapse_time_ms=0;` |
| kernel_timer.c | 15 | 256-256 | freertos | mot-clé | `#if (configSUPPORT_STATIC_ALLOCATION==1)` |
| kernel_timer.c | 16 | 258-258 | freertos | mot-clé | `while(xTimerStop(p_kernel_timer->timer, 10 )!=pdPASS);//disa` |
| kernel_timer.c | 17 | 261-261 | freertos | mot-clé | `while(xTimerChangePeriod(p_kernel_timer->timer,` |
| kernel_timer.c | 18 | 264-265 | freertos | mot-clé | `while(xTimerStart(p_kernel_timer->timer, 10 )!=pdPASS);` |
| kernel_timer.c | — | — | **fusion** | 0 conflit(s) | |
| process.c | 0 | 34-33 | ancien | lignée | `#include <stdarg.h>` |
| process.c | 1 | 63-65 | ancien | lignée | `#if defined (CPU_WIN32)` |
| process.c | 2 | 67-67 | ancien | lignée | `#endif` |
| process.c | 3 | 608-613 | ancien | lignée | `case F_DUPFD: {` |
| process.c | 4 | 616-651 | ancien | lignée | `case F_SETFD:` |
| process.c | 5 | 654-656 | ancien | lignée | `case F_GETFL:` |
| process.c | 6 | 707-714 | ancien | lignée | `//thread specific data` |
| process.c | 7 | 749-749 | ancien | lignée | `//` |
| process.c | 8 | 822-822 | ancien | lignée | `` |
| process.c | 9 | 825-825 | ancien | lignée | `` |
| process.c | 10 | 914-915 | freertos | mot-clé | `//freeRTOS temporary patch` |
| process.c | 11 | 1014-1020 | ancien | lignée | `//thread once mutex` |
| process.c | 12 | 1026-1046 | ancien | lignée | `//thread specific data` |
| process.c | 13 | 1051-1050 | ancien | lignée | `#endif` |
| process.c | 14 | 1202-1203 | freertos | mot-clé | `//freeRTOS temporary patch` |
| process.c | 15 | 1249-1255 | ancien | lignée | `//thread once mutex` |
| process.c | 16 | 1261-1281 | ancien | lignée | `//thread specific data` |
| process.c | 17 | 1286-1285 | ancien | lignée | `#endif` |
| process.c | 18 | 1386-1386 | ancien | lignée | `` |
| process.c | 19 | 1425-1425 | ancien | lignée | `//free main pthread of cureent process` |
| process.c | 20 | 1427-1432 | ancien | lignée | `//thread once mutex` |
| process.c | 21 | 1457-1457 | ancien | lignée | `//free main pthread of cureent process` |
| process.c | 22 | 1459-1464 | ancien | lignée | `//thread once mutex` |
| process.c | 23 | 1492-1492 | ancien | lignée | `//free main pthread of cureent process` |
| process.c | 24 | 1494-1499 | ancien | lignée | `//thread once mutex` |
| process.c | 25 | 1629-1630 | ancien | lignée | `if(sig<NSIG) {` |
| process.c | — | — | **fusion** | 0 conflit(s) | |
| signal.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| signal.c | 1 | 64-64 | ancien | lignée | `,{(sa_handler_t)SIG_IGN,{0,0},0,(sa_sigaction_t)0},  // SIGC` |
| signal.c | 2 | 78-78 | ancien | lignée | `{(sa_handler_t)SIG_IGN,{0,0},0,(sa_sigaction_t)0},   //     ` |
| signal.c | 3 | 133-134 | ancien | lignée | `` |
| signal.c | 4 | 142-159 | ancien | lignée | `case SIGTERM:` |
| signal.c | 5 | 405-405 | ancien | lignée | `#ifdef __KERNEL_POSIX_REALTIME_SIGNALS` |
| signal.c | 6 | 423-422 | ancien | lignée | `#else` |
| signal.c | 7 | 434-434 | ancien | lignée | `#ifdef __KERNEL_POSIX_REALTIME_SIGNALS` |
| signal.c | 8 | 453-452 | ancien | lignée | `#else` |
| signal.c | — | — | **fusion** | 0 conflit(s) | |
| syscall.c | 0 | 12-13 | entete | licence | `The Initial Developer of the Original Code is Philippe Le Bo` |
| syscall.c | 1 | 31-30 | ancien | lignée | `#include <string.h>` |
| syscall.c | 2 | 44-44 | ancien | lignée | `` |
| syscall.c | 3 | 172-172 | ancien | lignée | `//` |
| syscall.c | 4 | 348-347 | ancien | lignée | `//to do: kill_dt->pid==0 ?? nothing to do?` |
| syscall.c | 5 | 711-711 | ancien | lignée | `\| Name:_syscall_calloc` |
| syscall.c | 6 | 795-795 | ancien | lignée | `fcntl_dt->ret=-1;` |
| syscall.c | 7 | 851-851 | ancien | lignée | `va_list _ap;` |
| syscall.c | 8 | 871-871 | ancien | lignée | `ioctl_dt->ret = _vfs_ioctl(desc,ioctl_dt->request,desc_link,` |
| syscall.c | 9 | 917-919 | ancien | lignée | `__flush_syscall(pthread_ptr);` |
| syscall.c | 10 | 930-930 | ancien | lignée | `int _syscall_getpgrp(kernel_pthread_t* pthread_ptr, pid_t pi` |
| syscall.c | 11 | 946-946 | ancien | lignée | `int _syscall_pthread_create(kernel_pthread_t* pthread_ptr, p` |
| syscall.c | 12 | 949-950 | ancien | lignée | `pthread_create_dt->ret=_sys_pthread_create(&pthread_create_d` |
| syscall.c | 13 | 962-962 | ancien | lignée | `\| Comments:` |
| syscall.c | 14 | 965-965 | ancien | lignée | `int _syscall_pthread_cancel(kernel_pthread_t* pthread_ptr, p` |
| syscall.c | 15 | 968-968 | ancien | lignée | `if(process_lst[pid]->pthread_ptr!=pthread_cancel_dt->kernel_` |
| syscall.c | 16 | 976-976 | ancien | lignée | `pthread_cancel_dt->ret = _sys_pthread_cancel(pthread_cancel_` |
| syscall.c | 17 | 979-979 | ancien | lignée | `if(pthread_cancel_dt->kernel_pthread==_syscall_owner_pthread` |
| syscall.c | 18 | 986-987 | ancien | lignée | `}else{` |
| syscall.c | 19 | 993-993 | ancien | lignée | `return _syscall_exit(pthread_ptr,pid,&exit_dt);` |
| syscall.c | 20 | 1007-1007 | ancien | lignée | `int _syscall_pthread_kill(kernel_pthread_t* pthread_ptr, pid` |
| syscall.c | 21 | 1011-1011 | ancien | lignée | `pthread_kill_dt->ret=-1;` |
| syscall.c | 22 | 1014-1013 | ancien | lignée | `` |
| syscall.c | 23 | 1015-1019 | ancien | lignée | `if(pthread_kill_dt->kernel_pthread) {` |
| syscall.c | 24 | 1021-1036 | ancien | lignée | `pthread_kill_dt->ret = _sys_kill(pthread_kill_dt->kernel_pth` |
| syscall.c | 25 | 1038-1057 | ancien | lignée | `}else if(pthread_kill_dt->kernel_pthread) {` |
| syscall.c | 26 | 1080-1083 | ancien | lignée | `//it's a secondary thread` |
| syscall.c | 27 | 1089-1115 | ancien | lignée | `//it's a thread annexe` |
| syscall.c | 28 | 1117-1133 | ancien | lignée | `//` |
| syscall.c | 29 | 1137-1138 | ancien | lignée | `//it's the main thread` |
| syscall.c | 30 | 1148-1191 | ancien | lignée | `return 0;` |
| syscall.c | 31 | 1362-1362 | ancien | lignée | `kernel_object_t* kernel_object;` |
| syscall.c | 32 | 1366-1366 | ancien | lignée | `if(!sem_init_dt->name) { //anonymous semaphore` |
| syscall.c | 33 | 1368-1372 | ancien | lignée | `sem_init_dt->psem,` |
| syscall.c | 34 | 1374-1455 | ancien | lignée | `}` |
| syscall.c | 35 | 1472-1472 | ancien | lignée | `kernel_object_t* kernel_object=(kernel_object_t*)0;` |
| syscall.c | 36 | 1475-1509 | ancien | lignée | `kernel_object_t* kernel_object=sem_destroy_dt->psem;` |
| syscall.c | — | — | **fusion** | 0 conflit(s) | |

Totaux : ancien 175, entete 13, freertos 48
