/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOS_H
#define NOS_H

#include <stdint.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(NOS_GLOBALS)
 #define NOS_EXTERN
#else
 #define NOS_EXTERN     extern
#endif

#if defined(NOS_USE_CONFIG_FILE)
#include "nOSConfig.h"
#endif
  
#if !defined(NOS_CONFIG_DEBUG)
 #define NOS_CONFIG_DEBUG                       0
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_DEBUG is not defined (disabled by default)."
 #endif
#endif
  
#if !defined(NOS_CONFIG_SAFE)
 #define NOS_CONFIG_SAFE                        1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_SAFE is not defined (enabled by default)."
 #endif
#endif

#if !defined(NOS_CONFIG_MAX_THREAD_PRIO)
 #define NOS_CONFIG_MAX_THREAD_PRIO             15
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_MAX_THREAD_PRIO is not defined (default to 15)."
 #endif
#elif NOS_CONFIG_MAX_THREAD_PRIO > 255
 #error "nOSConfig.h: NOS_CONFIG_MAX_THREAD_PRIO can't be higher than 255."
#endif
  
#if !defined(NOS_CONFIG_SCHED_LOCK_ENABLE)
 #define NOS_CONFIG_SCHED_LOCK_ENABLE           1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_SCHED_LOCK_ENABLE is not defined (enabled by default)."
 #endif
#endif
  
#if !defined(NOS_CONFIG_SLEEP_ENABLE)
 #define NOS_CONFIG_SLEEP_ENABLE                1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_SLEEP_ENABLE is not defined (enabled by default)."
 #endif
#endif

#if !defined(NOS_CONFIG_THREAD_SUSPEND_ENABLE)
 #define NOS_CONFIG_THREAD_SUSPEND_ENABLE       1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_THREAD_SUSPEND_ENABLE is not defined (enabled by default)."
 #endif
#endif
  
#if !defined(NOS_CONFIG_THREAD_SET_PRIO_ENABLE)
 #define NOS_CONFIG_THREAD_SET_PRIO_ENABLE      1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_THREAD_SET_PRIO_ENABLE is not defined (enabled by default)."
 #endif
#endif

#if !defined(NOS_CONFIG_SEM_ENABLE)
 #define NOS_CONFIG_SEM_ENABLE                  1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_SEM_ENABLE is not defined (enabled by default)."
 #endif
#endif
  
#if !defined(NOS_CONFIG_MUTEX_ENABLE)
 #define NOS_CONFIG_MUTEX_ENABLE                1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_MUTEX_ENABLE is not defined (enabled by default)."
 #endif
#endif
  
#if !defined(NOS_CONFIG_FLAG_ENABLE)
 #define NOS_CONFIG_FLAG_ENABLE                 1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_FLAG_ENABLE is not defined (enabled by default)."
 #endif
#elif (NOS_CONFIG_FLAG_ENABLE > 0)
 #if !defined(NOS_CONFIG_FLAG_NB_BITS)
  #define NOS_CONFIG_FLAG_NB_BITS               16
  #if defined(NOS_USE_CONFIG_FILE)
   #warning "nOSConfig.h: NOS_CONFIG_FLAG_NB_BITS is not defined (default to 16)."
  #endif
 #elif (NOS_CONFIG_FLAG_NB_BITS != 8) && (NOS_CONFIG_FLAG_NB_BITS != 16) && (NOS_CONFIG_FLAG_NB_BITS != 32)
  #error "nOSConfig.h: NOS_CONFIG_FLAG_NB_BITS set to invalid value: can be set to 8, 16 or 32."
 #endif
#else
 #undef NOS_CONFIG_FLAG_NB_BITS
#endif
  
#if !defined(NOS_CONFIG_QUEUE_ENABLE)
 #define NOS_CONFIG_QUEUE_ENABLE                1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_QUEUE_ENABLE is not defined (enabled by default)."
 #endif
#endif
  
#if !defined(NOS_CONFIG_MEM_ENABLE)
 #define NOS_CONFIG_MEM_ENABLE                  1
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_MEM_ENABLE is not defined (enabled by default)."
 #endif
#elif (NOS_CONFIG_MEM_ENABLE > 0)
 #if !defined(NOS_CONFIG_MEM_SANITY_CHECK_ENABLE)
  #define NOS_CONFIG_MEM_SANITY_CHECK_ENABLE    1
  #if defined(NOS_USE_CONFIG_FILE)
   #warning "nOSConfig.h: NOS_CONFIG_MEM_SANITY_CHECK_ENABLE is not defined (enabled by default)."
  #endif
 #endif
#else
 #undef NOS_CONFIG_MEM_SANITY_CHECK_ENABLE
#endif

#if !defined(NOS_CONFIG_TIMER_ENABLE)
 #define NOS_CONFIG_TIMER_ENABLE                1
 #define NOS_CONFIG_TIMER_THREAD_PRIO           0
 #define NOS_CONFIG_TIMER_THREAD_STACK_SIZE     128
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_TIMER_ENABLE is not defined (enabled by default)."
 #endif
 #if (NOS_CONFIG_SEM_ENABLE == 0)
  #error "nOSConfig.h: NOS_CONFIG_SEM_ENABLE need to be enable when NOS_CONFIG_TIMER_ENABLE is enable."
 #endif
#elif (NOS_CONFIG_TIMER_ENABLE > 0)
 #if (NOS_CONFIG_SEM_ENABLE == 0)
  #error "nOSConfig.h: NOS_CONFIG_SEM_ENABLE need to be enable when NOS_CONFIG_TIMER_ENABLE is enable."
 #endif
 #if !defined(NOS_CONFIG_TIMER_THREAD_PRIO)
  #define NOS_CONFIG_TIMER_THREAD_PRIO          0
  #if defined(NOS_USE_CONFIG_FILE)
   #warning "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_PRIO is not defined (default to 0)."
  #endif
 #elif (NOS_CONFIG_TIMER_THREAD_PRIO > NOS_CONFIG_MAX_THREAD_PRIO)
  #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_PRIO is higher than NOS_CONFIG_MAX_THREAD_PRIO."
 #endif
 #if !defined(NOS_CONFIG_TIMER_THREAD_STACK_SIZE)
  #define NOS_CONFIG_TIMER_THREAD_STACK_SIZE    128
  #if defined(NOS_USE_CONFIG_FILE)
   #warning "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_STACK_SIZE is not defined (default to 128)."
  #endif
 #endif
#else
 #undef NOS_CONFIG_TIMER_THREAD_PRIO
 #undef NOS_CONFIG_TIMER_THREAD_STACK_SIZE
#endif

typedef struct _nOS_List        nOS_List;
typedef struct _nOS_Node        nOS_Node;
typedef struct _nOS_Thread      nOS_Thread;
typedef struct _nOS_Event       nOS_Event;
#if (NOS_CONFIG_SEM_ENABLE > 0)
typedef struct _nOS_Sem         nOS_Sem;
#endif
#if (NOS_CONFIG_MUTEX_ENABLE > 0)
typedef struct _nOS_Mutex       nOS_Mutex;
#endif
#if (NOS_CONFIG_QUEUE_ENABLE > 0)
typedef struct _nOS_Queue       nOS_Queue;
#endif
#if (NOS_CONFIG_FLAG_ENABLE > 0)
typedef struct _nOS_Flag        nOS_Flag;
typedef struct _nOS_FlagContext nOS_FlagContext;
typedef struct _nOS_FlagResult  nOS_FlagResult;
#if (NOS_CONFIG_FLAG_NB_BITS == 8)
#define nOS_FlagBits            uint8_t
#elif (NOS_CONFIG_FLAG_NB_BITS == 16)
#define nOS_FlagBits            uint16_t
#else   /* NOS_CONFIG_FLAG_NB_BITS == 32 */
#define nOS_FlagBits            uint32_t
#endif
#endif
#if (NOS_CONFIG_MEM_ENABLE > 0)
typedef struct _nOS_Mem         nOS_Mem;
#endif
#if (NOS_CONFIG_TIMER_ENABLE > 0)
typedef struct _nOS_Timer       nOS_Timer;
#endif

typedef enum _nOS_Error
{
    NOS_OK = 0,
    NOS_E_NULL = -1,
    NOS_E_INV_VAL = -2,
    NOS_E_LOCKED = -3,
    NOS_E_ISR = -4,
    NOS_E_IDLE = -5,
    NOS_E_TIMEOUT = -6,
    NOS_E_UNDERFLOW = -7,
    NOS_E_OVERFLOW = -8,
    NOS_E_AGAIN = -9,
    NOS_E_OWNER = -10,
    NOS_E_EMPTY = -11,
    NOS_E_FULL = -12
} nOS_Error;

#include "port.h"

struct _nOS_List
{
    nOS_Node        *head;
    nOS_Node        *tail;
};

struct _nOS_Node
{
    nOS_Node        *prev;
    nOS_Node        *next;
    void            *payload;
};

struct _nOS_Thread
{
    nOS_Stack       *stackPtr;
    uint8_t         prio;
    nOS_Error       error;
    uint8_t         state;
    uint16_t        timeout;
    nOS_Event       *event;
    void            *context;

    nOS_Node        full;
    nOS_Node        readyWaiting;
};

struct _nOS_Event
{
    nOS_List        waitingList;
};

#if (NOS_CONFIG_SEM_ENABLE > 0)
struct _nOS_Sem
{
    nOS_Event       e;
    uint16_t        count;
    uint16_t        max;
};
#endif

#if (NOS_CONFIG_MUTEX_ENABLE > 0)
struct _nOS_Mutex
{
    nOS_Event       e;
    uint8_t         type;
    uint8_t         count;
    uint8_t         prio;
    uint8_t         backup;
    nOS_Thread      *owner;
};
#endif

#if (NOS_CONFIG_QUEUE_ENABLE > 0)
struct _nOS_Queue
{
    nOS_Event       e;
    uint8_t         *buffer;
    uint16_t        bsize;
    uint16_t        max;
    uint16_t        count;
    uint16_t        r;
    uint16_t        w;
};
#endif

#if (NOS_CONFIG_FLAG_ENABLE > 0)
struct _nOS_Flag
{
    nOS_Event       e;
    nOS_FlagBits    flags;
};

struct _nOS_FlagContext
{
    uint8_t         opt;
    nOS_FlagBits    flags;
    nOS_FlagBits    *rflags;
};

struct _nOS_FlagResult
{
    uint8_t         sched;
    nOS_FlagBits    rflags;
};
#endif

#if (NOS_CONFIG_MEM_ENABLE > 0)
struct _nOS_Mem
{
    nOS_Event       e;
    void            **list;
    uint16_t        count;
#if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
    uint8_t         *buffer;
    uint16_t        bsize;
    uint16_t        max;
#endif
};
#endif

#if (NOS_CONFIG_TIMER_ENABLE > 0)
struct _nOS_Timer
{
    uint8_t         state;
    uint16_t        count;
    uint16_t        delay;
    void(*callback)(void*);
    void            *arg;
    nOS_Node        node;
};
#endif

#define NOS_PRIO_IDLE               0

#define NOS_NO_WAIT                 0
#define NOS_WAIT_INFINITE           UINT16_MAX

#define NOS_READY                   0x00
#define NOS_SLEEPING                0x01
#define NOS_TAKING_SEM              0x02
#define NOS_LOCKING_MUTEX           0x03
#define NOS_READING_QUEUE           0x04
#define NOS_WAITING_FLAG            0x05
#define NOS_MEM_ALLOC               0x06
#define NOS_WAITING                 0x0F
#define NOS_SUSPENDED               0x40
#define NOS_STOPPED                 0x80

#define NOS_SEM_COUNT_MAX           UINT16_MAX

#define NOS_MUTEX_NORMAL            0
#define NOS_MUTEX_RECURSIVE         1
#define NOS_MUTEX_PRIO_INHERIT      0

#define NOS_FLAG_WAIT               0x01
#define NOS_FLAG_WAIT_ANY           0x00
#define NOS_FLAG_WAIT_ALL           0x01
#define NOS_FLAG_CLEAR_ON_EXIT      0x02
#define NOS_FLAG_NONE               0

#define NOS_TIMER_ONE_SHOT          0x00
#define NOS_TIMER_FREE_RUNNING      0x01
#define NOS_TIMER_OPT               0x01
#define NOS_TIMER_RUNNING           0x80

#if defined(NOS_PRIVATE)
NOS_EXTERN nOS_Thread   nOS_mainThread;

NOS_EXTERN uint8_t      nOS_isrNestingCounter;
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
NOS_EXTERN uint8_t      nOS_lockNestingCounter;
#endif

NOS_EXTERN nOS_Thread   *nOS_runningThread;
NOS_EXTERN nOS_Thread   *nOS_highPrioThread;

NOS_EXTERN nOS_List     nOS_fullList;
NOS_EXTERN nOS_List     nOS_readyList[NOS_CONFIG_MAX_THREAD_PRIO+1];
#if defined(NOS_PORT_SCHED_USE_32_BITS)
#if NOS_CONFIG_MAX_THREAD_PRIO < 32
NOS_EXTERN uint32_t     nOS_readyPrio;
#elif NOS_CONFIG_MAX_THREAD_PRIO < 256
NOS_EXTERN uint32_t     nOS_readyGroup;
NOS_EXTERN uint32_t     nOS_readyPrio[8];
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
#else   /* NOS_PORT_SCHED_USE_32_BITS */
#if NOS_CONFIG_MAX_THREAD_PRIO < 8
NOS_EXTERN uint8_t      nOS_readyPrio;
#elif NOS_CONFIG_MAX_THREAD_PRIO < 16
NOS_EXTERN uint16_t     nOS_readyPrio;
#elif NOS_CONFIG_MAX_THREAD_PRIO < 64
NOS_EXTERN uint8_t      nOS_readyGroup;
NOS_EXTERN uint8_t      nOS_readyPrio[8];
#elif NOS_CONFIG_MAX_THREAD_PRIO < 256
NOS_EXTERN uint16_t     nOS_readyGroup;
NOS_EXTERN uint16_t     nOS_readyPrio[16];
#endif  /* NOS_CONFIG_MAX_THREAD_PRIO */
#endif  /* NOS_PORT_SCHED_USE_32_BITS */
#endif  /* NOS_PRIVATE */

#if defined(NOS_PRIVATE)
nOS_Thread* SchedHighPrio               (void);
void        AppendThreadToReadyList     (nOS_Thread *thread);
void        RemoveThreadFromReadyList   (nOS_Thread *thread);
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
void        SuspendThread               (void *payload, void *arg);
void        ResumeThread                (void *payload, void *arg);
#endif
void        SetThreadPriority           (nOS_Thread *thread, uint8_t prio);
void        TickThread                  (void *payload, void *arg);
void        SignalThread                (nOS_Thread  *thread);
#endif /* NOS_PRIVATE */

nOS_Error   nOS_Init                    (void);
nOS_Error   nOS_Sched                   (void);
nOS_Error   nOS_Yield                   (void);
void        nOS_Tick                    (void);
#if (NOS_CONFIG_SLEEP_ENABLE > 0)
nOS_Error   nOS_Sleep                   (uint16_t dly);
#endif

#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
nOS_Error   nOS_SchedLock               (void);
nOS_Error   nOS_SchedUnlock             (void);
#endif

void        nOS_ListInit                (nOS_List *list);
void*       nOS_ListHead                (nOS_List *list);
void        nOS_ListAppend              (nOS_List *list, nOS_Node *node);
void        nOS_ListRemove              (nOS_List *list, nOS_Node *node);
void        nOS_ListRotate              (nOS_List *list);
void        nOS_ListWalk                (nOS_List *list, void(*callback)(void*, void*), void *arg);

#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
nOS_Error   nOS_ThreadCreate            (nOS_Thread *thread, void(*func)(void*), void *arg, nOS_Stack *stack, size_t ssize, uint8_t prio, uint8_t state);
#else
nOS_Error   nOS_ThreadCreate            (nOS_Thread *thread, void(*func)(void*), void *arg, nOS_Stack *stack, size_t ssize, uint8_t prio);
#endif
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
nOS_Error   nOS_ThreadSuspend           (nOS_Thread *thread);
nOS_Error   nOS_ThreadSuspendAll        (void);
nOS_Error   nOS_ThreadResume            (nOS_Thread *thread);
nOS_Error   nOS_ThreadResumeAll         (void);
#endif
uint8_t     nOS_ThreadGetPriority       (nOS_Thread *thread);
nOS_Error   nOS_ThreadSetPriority       (nOS_Thread *thread, uint8_t prio);
nOS_Thread* nOS_ThreadRunning           (void);

void        nOS_EventCreate             (nOS_Event *event);
nOS_Error   nOS_EventWait               (nOS_Event *event, uint8_t state, uint16_t tout);
nOS_Thread* nOS_EventSignal             (nOS_Event *event);

#if (NOS_CONFIG_SEM_ENABLE > 0)
nOS_Error   nOS_SemCreate               (nOS_Sem *sem, uint16_t cntr, uint16_t max);
nOS_Error   nOS_SemTake                 (nOS_Sem *sem, uint16_t tout);
nOS_Error   nOS_SemGive                 (nOS_Sem *sem);
#endif

#if (NOS_CONFIG_MUTEX_ENABLE > 0)
nOS_Error   nOS_MutexCreate             (nOS_Mutex *mutex, uint8_t type, uint8_t prio);
nOS_Error   nOS_MutexLock               (nOS_Mutex *mutex, uint16_t tout);
nOS_Error   nOS_MutexUnlock             (nOS_Mutex *mutex);
#endif

#if (NOS_CONFIG_QUEUE_ENABLE > 0)
nOS_Error   nOS_QueueCreate             (nOS_Queue *queue, void *buffer, uint16_t bsize, uint16_t max);
nOS_Error   nOS_QueueRead               (nOS_Queue *queue, void *buffer, uint16_t tout);
nOS_Error   nOS_QueueWrite              (nOS_Queue *queue, void *buffer);
#endif

#if (NOS_CONFIG_FLAG_ENABLE > 0)
nOS_Error   nOS_FlagCreate              (nOS_Flag *flag, nOS_FlagBits flags);
nOS_Error   nOS_FlagWait                (nOS_Flag *flag, uint8_t opt, nOS_FlagBits flags, nOS_FlagBits *res, uint16_t tout);
nOS_Error   nOS_FlagSend                (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits mask);
#endif

#if (NOS_CONFIG_MEM_ENABLE > 0)
nOS_Error   nOS_MemCreate               (nOS_Mem *mem, void *buffer, size_t bsize, uint16_t max);
void*       nOS_MemAlloc                (nOS_Mem *mem, uint16_t tout);
nOS_Error   nOS_MemFree                 (nOS_Mem *mem, void *block);
#endif

#if (NOS_CONFIG_TIMER_ENABLE > 0)
void        nOS_TimerInit               (void);
void        nOS_TimerTick               (void);
nOS_Error   nOS_TimerCreate             (nOS_Timer *timer, void(*callback)(void*), void *arg, uint16_t delay, uint8_t opt);
nOS_Error   nOS_TimerStart              (nOS_Timer *timer);
nOS_Error   nOS_TimerStop               (nOS_Timer *timer);
nOS_Error   nOS_TimerCallback           (nOS_Timer *timer, void(*callback)(void*), void *arg);
nOS_Error   nOS_TimerReload             (nOS_Timer *timer, uint16_t delay, uint8_t opt);
uint8_t     nOS_TimerRunning            (nOS_Timer *timer);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* NOS_H */
