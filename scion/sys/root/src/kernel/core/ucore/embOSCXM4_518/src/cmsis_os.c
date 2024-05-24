/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
File    : cmsis_os.c
Purpose : embOS CMSIS OS interface
*/

#include "cmsis_os.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/
#ifndef MAX_TASKS
#define MAX_TASKS       (10)
#endif

#ifndef MAX_STACK_SIZE
#define MAX_STACK_SIZE  (512)
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define PRIO_OFFSET     (4)
#define PRIO_MIN_VALUE  (-3)
#define PRIO_MAX_VALUE  (9)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  void*       Dummy;  // Required to prevent MEMF to corrupt TCB after the MEMF block is released and before OS_TerminateTask() is called.
  OS_TASK     Task;
  OS_U8*      pStack;
  os_pthread  pFunction;
  const void* Argument;
} OS_TASK_INTERNAL;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static OS_MEMF MEMF_TCB;
static OS_U32  aTCBPool[(MAX_TASKS * sizeof(OS_TASK_INTERNAL) + 3) / 4];

static OS_MEMF MEMF_Stacks;
static OS_U32  aStackPool[(MAX_TASKS * MAX_STACK_SIZE + 3) / 4];


/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _ThreadWrapper()
*/
static void _ThreadWrapper(const void* arg) {
  OS_TASK_INTERNAL* pTaskInternal;

  pTaskInternal = (OS_TASK_INTERNAL*)arg;
  pTaskInternal->pFunction(pTaskInternal->Argument);
  osThreadTerminate(&(pTaskInternal->Task));
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

//  ==== Kernel Control Functions ====

/*********************************************************************
*
*       osKernelInitialize()
*/
osStatus osKernelInitialize(void) {
  if (OS_IsRunning() == 0u) {
    //
    // Init embOS and hardware
    //
    OS_InitKern();
    OS_InitHW();
    //
    // Create memory pools for task control blocks and task stacks
    //
    OS_MEMF_Create(&MEMF_TCB,    aTCBPool,   MAX_TASKS, sizeof(OS_TASK_INTERNAL));
    OS_MEMF_Create(&MEMF_Stacks, aStackPool, MAX_TASKS, MAX_STACK_SIZE);
    return osOK;
  } else {
    //
    // embOS is already started, don't call osKernelInitialize() after embOS start
    //
    return osErrorOS;
  }
}

/*********************************************************************
*
*       osKernelStart()
*/
osStatus osKernelStart(void)  {
  if (OS_InInterrupt() != 0u) {         // cannot call from ISR
    return osErrorISR;
  } else if (OS_GetTaskID() != NULL) {  // cannot call OS_Start() from Task
    return osErrorOS;
  } else if (OS_IsRunning() != 0u) {    // cannot call OS_Start() more than once
    return osErrorOS;
  }
  OS_Start();
  return osOK;
}

/*********************************************************************
*
*       osKernelRunning()
*/
int32_t osKernelRunning(void) {
  return (int32_t)OS_IsRunning();
}

/*********************************************************************
*
*       osKernelSysTick()
*/
uint32_t osKernelSysTick(void) {
  return (uint32_t)OS_GetTime32();
}

//  ==== Thread Management ====

/*********************************************************************
*
*       osThreadCreate()
*/
osThreadId osThreadCreate(const osThreadDef_t* thread_def, void* argument) {
  OS_TASK_INTERNAL* pTaskInternal;
  OS_U32            StackSize;
  OS_U8*            pStack;

  if (OS_InInterrupt() != 0u) {         // cannot call from ISR
    return NULL;
  }
  //
  // Check parameter
  //
  if ((thread_def == NULL) ||
      (thread_def->pthread == NULL) ||
      (thread_def->tpriority < osPriorityIdle) ||
      (thread_def->tpriority > osPriorityRealtime)) {
    return NULL;
  }
  //
  // Set stack size
  //
  if ((thread_def->stacksize == 0) || (thread_def->stacksize > MAX_STACK_SIZE)) {
    StackSize = MAX_STACK_SIZE;
  } else {
    StackSize = thread_def->stacksize;
  }
  //
  // Get memory for task control block
  //
  pTaskInternal = OS_MEMF_Request(&MEMF_TCB, 0);
  if (pTaskInternal == NULL) {
    return NULL;
  }
  //
  // Get memory for task stack
  //
  pStack = OS_MEMF_Request(&MEMF_Stacks, 0);
  if (pStack == NULL) {
    OS_MEMF_Release(&MEMF_TCB, pTaskInternal);  // Free memory for task control block
    return NULL;
  }
  pTaskInternal->pStack = pStack;  // Save original stack address used to release the allocated memory
  pStack               += 4;       // Adjust end of stack by 4 bytes, since these are written when the allocated stack memory is released and would result in a false positive during stack check
  StackSize            -= 4;       // Also, adjust stack size accordingly
  //
  // Create the task
  //
  pTaskInternal->pFunction = thread_def->pthread;
  pTaskInternal->Argument  = argument;
  OS_CreateTaskEx(&(pTaskInternal->Task), "", thread_def->tpriority + PRIO_OFFSET, (void (*)(void*))_ThreadWrapper, pStack, StackSize, 2, (void*)pTaskInternal);
  return (osThreadId)&(pTaskInternal->Task);  // Return address of the task, not the internal task
}

/*********************************************************************
*
*       osThreadTerminate()
*/
osStatus osThreadTerminate(osThreadId thread_id) {
  OS_TASK_INTERNAL* pTaskInternal;
  char              r;

  if (OS_InInterrupt() != 0u) {         // cannot call from ISR
    return osErrorISR;
  }
  //
  // Check parameter
  //
  pTaskInternal = (OS_TASK_INTERNAL*)((unsigned long)thread_id - sizeof(void*));
  if (pTaskInternal == NULL) {
    return osErrorParameter;
  }
  r = OS_IsTask(&(pTaskInternal->Task));
  if (r == 0) {
    return osErrorParameter;
  }
  //
  // Free memory
  //
  OS_EnterRegion();
  OS_IncDI();
  //
  // OS_MEMF_Release writes() to the first word of the released memory block.
  // The allocated memory blocks contain dummy words, so that the TCB and stack won't
  // be overwritten and are still intact for the scheduler to terminate the task.
  //
  OS_MEMF_Release(&MEMF_Stacks, (void*)pTaskInternal->pStack);  // Free memory for task stack
  OS_MEMF_Release(&MEMF_TCB,    (void*)pTaskInternal);          // Free memory for task control block
  //
  // Terminate the task
  //
  OS_TerminateTask(thread_id);
  OS_DecRI();
  OS_LeaveRegion();
  return osOK;
}

/*********************************************************************
*
*       osThreadYield()
*/
osStatus osThreadYield(void) {
  if (OS_InInterrupt() != 0u) {         // cannot call from ISR
    return osErrorISR;
  } else if (OS_GetTaskID() == NULL) {  // cannot call from main() and OS_Idle()
    return osErrorOS;
  }
  OS_Yield();
  return osOK;
}

/*********************************************************************
*
*       osThreadGetId()
*/
osThreadId osThreadGetId(void) {
  osThreadId Id;

  if (OS_InInterrupt() != 0u) {
    Id = NULL;
  } else {
    Id = OS_Global.pCurrentTask;
  }
  return Id;
}

/*********************************************************************
*
*       osThreadSetPriority()
*/
osStatus osThreadSetPriority(osThreadId thread_id, osPriority priority) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (thread_id == NULL) {
    return osErrorParameter;
  }
  if ((priority < PRIO_MIN_VALUE) || (priority > PRIO_MAX_VALUE)) {
    return osErrorValue;
  }
  OS_SetPriority(thread_id, priority + PRIO_OFFSET);
  return osOK;
}

/*********************************************************************
*
*       osThreadGetPriority()
*/
osPriority osThreadGetPriority(osThreadId thread_id) {
  char r;
  int  prio;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osPriorityError;
  }
  r = OS_IsTask(thread_id);
  if (r == 0) {
    return osPriorityError;
  }
  prio = ((int)OS_GetPriority(thread_id)) - PRIO_OFFSET;
  return (osPriority)prio;
}


//  ==== Generic Wait Functions ====

/*********************************************************************
*
*       osDelay()
*/
osStatus osDelay(uint32_t millisec) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  OS_Delay(millisec);
  return osEventTimeout;
}

/*********************************************************************
*
*       osWait()
*/
osEvent osWait(uint32_t millisec) {
  osEvent e;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    e.status = osErrorISR;
  }
  OS_Delay(millisec);
  e.status = osEventTimeout;
  return e;
}

//  ==== Timer Management Functions ====

/*********************************************************************
*
*       _osTimerCallBack()
*/
static void _osTimerCallBack(void) {
  os_timer_cmsis_def_t* t;

  t = (os_timer_cmsis_def_t*)OS_GetpCurrentTimerEx();
  //
  // Call user function
  //
  (*(t->pTimerFunc))(t->argument);
  //
  // Check if timer is periodic, if yes, re-trigger it
  //
  if (t->type == osTimerPeriodic) {
    OS_RetriggerTimer(&(t->Timer));
  }
}

/*********************************************************************
*
*       osTimerCreate()
*/
osTimerId osTimerCreate(const osTimerDef_t* timer_def, os_timer_type type, void* argument) {
  osTimerDef_t* t;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return NULL;
  }
  if (timer_def == NULL) {
    return NULL;
  }
  t = (osTimerDef_t*)timer_def;
  t->Timer.argument   = argument;
  t->Timer.pTimerFunc = t->pTimerFunc;
  t->Timer.type       = type;
  OS_CreateTimer ((OS_TIMER*)&(t->Timer), (OS_TIMERROUTINE*)_osTimerCallBack, 1);
  return &(t->Timer.Timer);
}

/*********************************************************************
*
*       osTimerStart()
*/
osStatus osTimerStart(osTimerId timer_id, uint32_t millisec) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (timer_id == NULL) {
    return osErrorParameter;
  }
  OS_SetTimerPeriod(timer_id, millisec);
  OS_RetriggerTimer(timer_id);
  return osOK;
}

/*********************************************************************
*
*       osTimerStop()
*/
osStatus osTimerStop(osTimerId timer_id) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (timer_id == NULL) {
    return osErrorParameter;
  }
  if (OS_GetTimerStatus(timer_id) == 0u) {
    return osErrorResource;
  }
  OS_StopTimer(timer_id);
  return osOK;
}

/*********************************************************************
*
*       osTimerDelete()
*/
osStatus osTimerDelete(osTimerId timer_id) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (timer_id == NULL) {
    return osErrorParameter;
  }
  OS_DeleteTimer(timer_id);
  return osOK;
}

//  ==== Signal Management ====

/*********************************************************************
*
*       osSignalSet()
*/
int32_t osSignalSet(osThreadId thread_id, int32_t signals) {
  int32_t r;

  if (thread_id == NULL) {     // Check thread id
    return ((int32_t)0x80000000);
  }
  if (signals & 0x80000000) {  // Check signal parameter, 31 signals available
    return ((int32_t)0x80000000);
  }

  r = OS_GetEventsOccurred(thread_id);
  OS_SignalEvent(signals, thread_id);
  return r;
}

/*********************************************************************
*
*       osSignalClear()
*/
int32_t osSignalClear(osThreadId thread_id, int32_t signals) {
  int32_t r;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return 0x80000000;
  }
  if (thread_id == NULL) {       // Check thread id
    return ((int32_t)0x80000000);
  }
  if (signals & 0x80000000) {    // Check signal parameter, 31 signals available
    return ((int32_t)0x80000000);
  }

#if (OS_VERSION < 43000u)
  return 0x80000000;             // Not supported with embOS version before V430
#else
  r = OS_GetEventsOccurred(thread_id);
  OS_ClearEventsEx(thread_id, signals);
#endif
  return r;
}

/*********************************************************************
*
*       osSignalWait()
*/
osEvent osSignalWait(int32_t signals, uint32_t millisec) {
  osEvent      e;
  unsigned int tEnd;
  unsigned int t;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    e.status = osErrorISR;
    return e;
  }

  //
  // 0x80000000 is outside of the permitted range.
  //
  if ((signals & (1u << osFeature_Signals)) != 0x00) {
    e.status = osErrorValue;
    return e;
  }

  if (millisec == 0u) {
    e.value.v = OS_GetEventsOccurred(NULL);
    if (e.value.v == 0u) {
      e.status = osOK;
    } else {
      e.status = osEventSignal;
    }
  } else {
    if (millisec == osWaitForever) {
      if (signals == 0) {
        //
        // Wait for any signal
        //
        signals = 0xFFFFFFFFu;
      }
      e.value.v = OS_WaitEvent(signals);
      e.status = osEventSignal;
    } else {
      if (signals == 0) {
        e.value.v = OS_WaitEvent_Timed(0xFFFFFFFFu, millisec);  // Wait for any signal
        if (e.value.v == 0) {
          e.status = osEventTimeout;
        } else {
          e.status = osEventSignal;
        }
      } else {
        e.value.v = 0;
        tEnd      = OS_GetTime() + millisec;
        while ((e.value.v & signals) != (uint32_t)signals) {
          t = OS_GetTime();
          if (tEnd <= t) {
            break;
          }
          e.value.v |= OS_WaitEvent_Timed(0xFFFFFFFFu, tEnd - t);
        }
      }
      if (e.value.v == 0) {
        e.status = osEventTimeout;
      } else {
        e.status = osEventSignal;
      }
    }
  }
  return e;
}


//  ==== Mutex Management ====

/*********************************************************************
*
*       osMutexCreate()
*/
osMutexId osMutexCreate(const osMutexDef_t* mutex_def) {
  OS_RSEMA *pRSema;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return NULL;
  }
  pRSema = (OS_RSEMA*)&(mutex_def->rsema);
  OS_CREATERSEMA(pRSema);
  return pRSema;
}

/*********************************************************************
*
*       osMutexWait()
*/
osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec) {
  int r;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (mutex_id == NULL) {
    return osErrorParameter;
  }
  if (millisec == 0) {
    r = OS_Request(mutex_id);
    if (r == 0) {
      return osErrorResource;
    }
  } else {
    if (millisec == osWaitForever) {
      r = OS_Use(mutex_id);
    } else {
      r = OS_UseTimed(mutex_id, millisec);
      if (r == 0) {
        return osErrorTimeoutResource;
      }
    }
  }
  return osOK;
}

/*********************************************************************
*
*       osMutexRelease()
*/
osStatus osMutexRelease(osMutexId mutex_id) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (mutex_id == NULL) {
    return osErrorParameter;
  }
  if (OS_GetSemaValue(mutex_id) == 0u) {
    return osErrorResource;
  }
  if (((OS_RSEMA  *)mutex_id)->pTask != OS_GetTaskID()) {
    return osErrorResource;
  }
  OS_Unuse(mutex_id);
  return osOK;
}

/*********************************************************************
*
*       osMutexDelete()
*/
osStatus osMutexDelete(osMutexId mutex_id) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (mutex_id == NULL) {
    return osErrorParameter;
  }
  if (OS_GetResourceOwner(mutex_id) == NULL) {
    OS_DeleteRSema(mutex_id);
    return osOK;
  } else {
    return osErrorResource;
  }
}

//  ==== Semaphore Management Functions ====

/*********************************************************************
*
*       osSemaphoreCreate()
*/
osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t* semaphore_def, int32_t count) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return NULL;
  }
  OS_CreateCSema((OS_CSEMA *)&(semaphore_def->csema), count);
  return (osSemaphoreId)&(semaphore_def->csema);
}

/*********************************************************************
*
*       osSemaphoreWait()
*/
int32_t osSemaphoreWait(osSemaphoreId semaphore_id, uint32_t millisec) {
  int     r;
  int32_t CSemaValue;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return -1;
  }
  if (semaphore_id == NULL) {
    return -1;
  }
  if (millisec == 0u) {
    r = OS_CSemaRequest(semaphore_id);
    if (r == 0) {
      return 0u;
    }
  } else {
    if (millisec == osWaitForever ) {
      OS_WaitCSema(semaphore_id);
    } else {
      r = OS_WaitCSemaTimed(semaphore_id, millisec);
      if (r == 0) {
        return 0u;
      }
    }
  }
  CSemaValue = OS_GetCSemaValue(semaphore_id) + 1;
  return CSemaValue;
}

/*********************************************************************
*
*       osSemaphoreRelease()
*/
osStatus osSemaphoreRelease(osSemaphoreId semaphore_id) {
  int CSemaValue;

  if (semaphore_id == NULL) {
    return osErrorParameter;
  }

  CSemaValue = OS_GetCSemaValue(semaphore_id);
  if (CSemaValue == 0xFFFF) {
    return osErrorResource;
  }

  if ((unsigned int)CSemaValue >= osFeature_Semaphore) {
    return osErrorResource;
  }

  OS_SignalCSema(semaphore_id);
  return osOK;
}

/*********************************************************************
*
*       osSemaphoreDelete()
*/
osStatus osSemaphoreDelete(osSemaphoreId semaphore_id) {
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return osErrorISR;
  }
  if (semaphore_id == NULL) {
    return osErrorParameter;
  }
  OS_DeleteCSema(semaphore_id);
  return osOK;
}


//  ==== Memory Pool Management Functions ====

/*********************************************************************
*
*       osPoolCreate()
*/
osPoolId osPoolCreate(const osPoolDef_t* pool_def) {
  OS_MEMF* pMEMF;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return NULL;
  }
  if (pool_def == NULL) {
    return NULL;
  }
  pMEMF = (OS_MEMF*)&(pool_def->memf);
  OS_MEMF_Create(pMEMF, pool_def->pool, pool_def->pool_sz, pool_def->item_sz);
  return pMEMF;
}

/*********************************************************************
*
*       osPoolAlloc()
*/
void* osPoolAlloc(osPoolId pool_id) {
  if (pool_id == NULL) {
    return NULL;
  }
  return OS_MEMF_Request(pool_id, 0);
}

/*********************************************************************
*
*       osPoolCAlloc()
*/
void* osPoolCAlloc(osPoolId pool_id) {
  void*  p;
  char*  c;
  OS_U32 i;

  if (pool_id == NULL) {
    return NULL;
  }
  p = OS_MEMF_Request(pool_id, 0);
  //
  // Fill block with zeros
  //
  if (p != NULL) {
    c = (char*)p;
    for (i = 0; i < pool_id->BlockSize; i++) {
      *(c + i) = 0x00;
    }
  }
  return p;
}

/*********************************************************************
*
*       osPoolFree()
*/
osStatus osPoolFree(osPoolId pool_id, void* block) {
  char r;

  if (block == NULL || pool_id == NULL) {
    return osErrorParameter;
  }
  r = OS_MEMF_IsInPool(pool_id, block);
  if (r == 0) {
    return osErrorValue;
  }

  OS_MEMF_FreeBlock(block);
  return osOK;
}

//  ==== Message Queue Management Functions ====

/*********************************************************************
*
*       osMessageCreate()
*/
osMessageQId osMessageCreate(const osMessageQDef_t* queue_def, osThreadId thread_id) {
  OS_MAILBOX* pMB;

  (void)thread_id;
  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return NULL;
  }
  if (queue_def == NULL) {
    return NULL;
  }

  pMB = (OS_MAILBOX *)&(queue_def->mb);
  OS_CREATEMB(pMB, queue_def->item_sz, queue_def->queue_sz, queue_def->pool);

  return pMB;
}

/*********************************************************************
*
*       osMessagePut()
*/
osStatus osMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec) {
  char r;

  if (queue_id == NULL) {
    return osErrorParameter;
  }

#if (OS_VERSION < 43200u)
  if ((millisec != 0u) && (millisec != osWaitForever)) {
    return osErrorTimeoutResource;  // Not supported with embOS before V432
  }
#endif

  if (millisec == 0u) {
    r = OS_PutMailCond(queue_id, &info);
    if (r == 1u) {
      return osErrorResource;
    }
  } else {
    if (OS_InInterrupt() != 0u) {   // cannot call from ISR
      return osErrorParameter;
    }
    if (millisec == osWaitForever) {
      OS_PutMail(queue_id, (void*)&info);
    }
#if (OS_VERSION >= 43200u)
    else {
      r = OS_PutMailTimed(queue_id, (void*)&info, millisec);
      if (r == 1u) {
        return osErrorTimeoutResource;
      }
    }
#endif
  }
  return osOK;
}

/*********************************************************************
*
*       osMessageGet()
*/
osEvent osMessageGet(osMessageQId queue_id, uint32_t millisec) {
  osEvent e;
  char    r;

  if (queue_id == NULL) {
    e.status = osErrorParameter;
    return e;
  }

  if (millisec == 0u) {
    r = OS_GetMailCond(queue_id, &(e.value.v));
    if (r == 0) {
      e.status = osEventMessage;
    } else {
      e.status = osOK;
    }
  } else {
    if (OS_InInterrupt() != 0u) {  // cannot call from ISR
      e.status = osErrorParameter;
      return e;
    }
    if (millisec == osWaitForever) {
      OS_GetMail(queue_id, &(e.value.v));
      e.status = osEventMessage;
    } else {
      r = OS_GetMailTimed(queue_id, &(e.value.v), millisec);
      if (r == 0) {
        e.status = osEventMessage;
      } else {
        e.status = osEventTimeout;
      }
    }
  }
  e.value.p = (void *)(e.value.v);
  return e;
}

//  ==== Mail Queue Management Functions ====

/*********************************************************************
*
*       osMailCreate()
*/
osMailQId osMailCreate(const osMailQDef_t* queue_def, osThreadId thread_id) {
  OS_MEMF*      pMEMF;
  OS_Q*         pQueue;
  osMailQDef_t* pMail;

  if (OS_InInterrupt() != 0u) {  // cannot call from ISR
    return NULL;
  }
  if (queue_def == NULL) {
    return NULL;
  }
  (void) thread_id;
  pMEMF   = (OS_MEMF*)&(queue_def->memf);
  pQueue  = (OS_Q*)&(queue_def->queue);
  pMail   = (osMailQDef_t*)queue_def;
  OS_MEMF_Create(pMEMF, (void*)queue_def->pMemfBuffer, queue_def->queue_sz, queue_def->item_sz);
  OS_Q_Create(pQueue, queue_def->pQueueBuffer, queue_def->queue_sz * sizeof(void*));

  return pMail;
}

/*********************************************************************
*
*       osMailAlloc()
*/
void* osMailAlloc(osMailQId queue_id, uint32_t millisec) {
  void* p;

  if (queue_id == NULL) {
    return NULL;
  }
  if (millisec == 0) {
    p = OS_MEMF_Request(&(queue_id->memf), 0);
  } else {
    if (OS_InInterrupt() != 0u) {  // cannot call from ISR
      return NULL;
    }
    if (millisec == osWaitForever) {
      p = OS_MEMF_Alloc(&(queue_id->memf), 0);
    } else {
      p = OS_MEMF_AllocTimed(&(queue_id->memf), millisec, 0);
    }
  }

  return p;
}

/*********************************************************************
*
*       osMailCAlloc()
*/
void* osMailCAlloc(osMailQId queue_id, uint32_t millisec) {
  void* p;
  char* c;
  OS_U32 i;

  if (queue_id == NULL) {
    return NULL;
  }
  if (millisec == 0) {
    p = OS_MEMF_Request(&(queue_id->memf), 0);
  } else {
    if (OS_InInterrupt() != 0u) {  // cannot call from ISR
      return NULL;
    }
    if (millisec == osWaitForever) {
      p = OS_MEMF_Alloc(&(queue_id->memf), 0);
    } else {
      p = OS_MEMF_AllocTimed(&(queue_id->memf), millisec, 0);
    }
  }

  if (p != NULL) {
    c = (char*)p;
    for (i = 0; i < queue_id->item_sz; i++) {
      *(c + i) = 0x00;
    }
  }

  return p;
}

/*********************************************************************
*
*       osMailPut()
*/
osStatus osMailPut(osMailQId queue_id, void* mail) {
  if (queue_id == NULL) {
    return osErrorParameter;
  }
  if (mail == NULL || (OS_MEMF_IsInPool(&(queue_id->memf), mail) == 0)) {
    return osErrorValue;
  }
  OS_Q_Put(&(queue_id->queue), &mail, sizeof(void*));
  return osOK;
}

/*********************************************************************
*
*       osMailGet()
*/
osEvent osMailGet(osMailQId queue_id, uint32_t millisec) {
  osEvent e;
  int     r;
  void*   ppData;

  if (queue_id == NULL) {
    e.status = osErrorParameter;
    return e;
  }
  if (OS_Q_IsInUse(&(queue_id->queue)) == 1) {
    e.status = osErrorResource;
    return e;
  }
  if (millisec == 0) {
    r = OS_Q_GetPtrCond(&(queue_id->queue), &ppData);
    if (r == 0) {
      e.status = osOK;
    } else {
      e.status = osEventMail;
      memcpy(&(e.value.p), ppData, sizeof(void*));
      OS_Q_Purge(&(queue_id->queue));
    }
  } else {
    if (millisec == osWaitForever) {
      OS_Q_GetPtr(&(queue_id->queue), &ppData);
      e.status = osEventMail;
      memcpy(&(e.value.p), ppData, sizeof(void*));
      OS_Q_Purge(&(queue_id->queue));
    } else {
      r = OS_Q_GetPtrTimed(&(queue_id->queue), &ppData, millisec);
      if (r == 0) {
        e.status = osEventTimeout;
      } else {
        e.status = osEventMail;
        memcpy(&(e.value.p), ppData, sizeof(void*));
        OS_Q_Purge(&(queue_id->queue));
      }
    }
  }
  return e;
}

/*********************************************************************
*
*       osMailFree()
*/
osStatus osMailFree(osMailQId queue_id, void *mail) {
  void* pDummy;
  int   r;

  if (queue_id == NULL) {
    return osErrorParameter;
  }
  if (mail == NULL || (OS_MEMF_IsInPool(&(queue_id->memf), mail) == 0)) {
    return osErrorValue;
  }
  if (OS_Q_IsInUse(&(queue_id->queue)) == 0) {         // if queue is not in use, purging is not allowed
    r = OS_Q_GetPtrCond(&(queue_id->queue), &pDummy);  // mark queue in use
    if (r > 0) {
      OS_Q_Purge(&(queue_id->queue));                  // purge message before memory is freed
    }
  }
  OS_MEMF_Release(&(queue_id->memf), mail);            // release allocated memory
  return osOK;
}

/*************************** End of file ****************************/
