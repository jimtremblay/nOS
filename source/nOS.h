/*
 * nOS v0.1
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOS_H_
#define NOS_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NOS_GLOBALS
 #define NOS_EXTERN
#else
 #define NOS_EXTERN     extern
#endif

#ifndef NOS_HIGHEST_PRIO
 #define NOS_HIGHEST_PRIO           15
#elif NOS_HIGHEST_PRIO > 255
 #error "nOS_Cfg.h: NOS_HIGHEST_PRIO can't be higher than 255."
#endif

#ifndef NOS_SAFE
 #define NOS_SAFE                   0
#endif

typedef struct _nOS_List        nOS_List;
typedef struct _nOS_Node        nOS_Node;
typedef struct _nOS_Thread      nOS_Thread;
typedef struct _nOS_Event       nOS_Event;
typedef struct _nOS_Sem         nOS_Sem;
typedef struct _nOS_Mutex       nOS_Mutex;
typedef struct _nOS_Queue       nOS_Queue;
typedef struct _nOS_Flag        nOS_Flag;
typedef struct _nOS_FlagContext nOS_FlagContext;
typedef struct _nOS_FlagResult  nOS_FlagResult;
typedef struct _nOS_Mem         nOS_Mem;

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
    uint8_t         *stackPtr;
    uint8_t         prio;
    int8_t          error;
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

struct _nOS_Sem
{
    nOS_Event       e;
    uint16_t        count;
    uint16_t        max;
};

struct _nOS_Mutex
{
    nOS_Event       e;
    uint8_t         type;
    uint8_t         count;
    uint8_t         prio;
    uint8_t         backup;
    nOS_Thread      *owner;
};

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

struct _nOS_Flag
{
    nOS_Event       e;
    unsigned int    flags;
};

struct _nOS_FlagContext
{
    uint8_t         opt;
    unsigned int    flags;
    unsigned int    *rflags;
};

struct _nOS_FlagResult
{
    unsigned int    rflags;
    uint8_t         sched;
};

struct _nOS_Mem
{
    nOS_Event       e;
    void            **list;
    uint16_t        max;
    uint16_t        count;
};

#include "port.h"

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

#define NOS_OK                      0
#define NOS_E_NULL                  -1
#define NOS_E_INV_VAL               -2
#define NOS_E_LOCKED                -3
#define NOS_E_ISR                   -4
#define NOS_E_IDLE                  -5
#define NOS_E_UNDERFLOW             -6
#define NOS_E_OVERFLOW              -7
#define NOS_E_AGAIN                 -8
#define NOS_E_OWNER                 -9
#define NOS_E_TIMEOUT               -10
#define NOS_E_EMPTY                 -11
#define NOS_E_FULL                  -12

#define NOS_MUTEX_NORMAL            0
#define NOS_MUTEX_RECURSIVE         1
#define NOS_MUTEX_PRIO_INHERIT      0

#define NOS_FLAG_WAIT               0x01
#define NOS_FLAG_WAIT_ANY           0x00
#define NOS_FLAG_WAIT_ALL           0x01
#define NOS_FLAG_CLEAR_ON_EXIT      0x02
#define NOS_FLAG_NONE               0

#ifdef NOS_PRIVATE
NOS_EXTERN nOS_Thread   nOS_mainThread;

NOS_EXTERN uint8_t      nOS_isrNestingCounter;
NOS_EXTERN uint8_t      nOS_lockNestingCounter;

NOS_EXTERN nOS_Thread   *nOS_runningThread;
NOS_EXTERN nOS_Thread   *nOS_highPrioThread;

NOS_EXTERN nOS_List     nOS_fullList;
NOS_EXTERN nOS_List     nOS_readyList[NOS_HIGHEST_PRIO+1];
#if NOS_HIGHEST_PRIO < 8
NOS_EXTERN uint8_t      nOS_readyPrio;
#elif NOS_HIGHEST_PRIO < 16
NOS_EXTERN uint16_t     nOS_readyPrio;
#elif NOS_HIGHEST_PRIO < 64
NOS_EXTERN uint8_t      nOS_readyGroup;
NOS_EXTERN uint8_t      nOS_readyPrio[8];
#elif NOS_HIGHEST_PRIO < 256
NOS_EXTERN uint16_t     nOS_readyGroup;
NOS_EXTERN uint16_t     nOS_readyPrio[16];
#endif
#endif  /* NOS_PRIVATE */

#ifdef NOS_PRIVATE
nOS_Thread* SchedHighPrio               (void);
void        AppendThreadToReadyList     (nOS_Thread *thread);
void        RemoveThreadFromReadyList   (nOS_Thread *thread);
void        SuspendThread               (void *payload, void *arg);
void        ResumeThread                (void *payload, void *arg);
void        SetThreadPriority           (nOS_Thread *thread, uint8_t prio);
void        TickThread                  (void *payload, void *arg);
void        SignalThread                (nOS_Thread  *thread);
#endif /* NOS_PRIVATE */

int8_t      nOS_Init                    (void);
int8_t      nOS_Sched                   (void);
int8_t      nOS_Yield                   (void);
void        nOS_Tick                    (void);
int8_t      nOS_Sleep                   (uint16_t dly);

int8_t      nOS_SchedLock               (void);
int8_t      nOS_SchedUnlock             (void);

void        nOS_ListInit                (nOS_List *list);
void*       nOS_ListHead                (nOS_List *list);
void        nOS_ListAppend              (nOS_List *list, nOS_Node *node);
void        nOS_ListRemove              (nOS_List *list, nOS_Node *node);
void        nOS_ListRotate              (nOS_List *list);
void        nOS_ListWalk                (nOS_List *list, void(*callback)(void*, void*), void *arg);

int8_t      nOS_ThreadCreate            (nOS_Thread *thread, void(*func)(void*), void *arg, uint8_t *sp, size_t ssize, uint8_t prio, uint8_t state);
int8_t      nOS_ThreadDelete            (nOS_Thread *thread);
int8_t      nOS_ThreadSuspend           (nOS_Thread *thread);
int8_t      nOS_ThreadSuspendAll        (void);
int8_t      nOS_ThreadResume            (nOS_Thread *thread);
int8_t      nOS_ThreadResumeAll         (void);
uint8_t     nOS_ThreadGetPriority       (nOS_Thread *thread);
int8_t      nOS_ThreadSetPriority       (nOS_Thread *thread, uint8_t prio);
nOS_Thread* nOS_ThreadRunning           (void);

void        nOS_EventCreate             (nOS_Event *event);
int8_t      nOS_EventWait               (nOS_Event *event, uint8_t state, uint16_t tout);
nOS_Thread* nOS_EventSignal             (nOS_Event *event);

int8_t      nOS_SemCreate               (nOS_Sem *sem, uint16_t cntr, uint16_t max);
int8_t      nOS_SemTake                 (nOS_Sem *sem, uint16_t tout);
int8_t      nOS_SemGive                 (nOS_Sem *sem);

int8_t      nOS_MutexCreate             (nOS_Mutex *mutex, uint8_t type, uint8_t prio);
int8_t      nOS_MutexLock               (nOS_Mutex *mutex, uint16_t tout);
int8_t      nOS_MutexUnlock             (nOS_Mutex *mutex);

int8_t      nOS_QueueCreate             (nOS_Queue *queue, void *buffer, uint16_t bsize, uint16_t max);
int8_t      nOS_QueueRead               (nOS_Queue *queue, void *buffer, uint16_t tout);
int8_t      nOS_QueueWrite              (nOS_Queue *queue, void *buffer);

int8_t      nOS_FlagCreate              (nOS_Flag *flag, unsigned int flags);
int8_t      nOS_FlagWait                (nOS_Flag *flag, uint8_t opt, unsigned int flags, unsigned int *res, uint16_t tout);
int8_t      nOS_FlagSet                 (nOS_Flag *flag, unsigned int flags, unsigned int mask);

int8_t      nOS_MemCreate               (nOS_Mem *mem, void *buffer, size_t bsize, uint16_t max);
void*       nOS_MemAlloc                (nOS_Mem *mem, uint16_t tout);
int8_t      nOS_MemFree                 (nOS_Mem *mem, void *block);

#ifdef __cplusplus
}
#endif

#endif /* NOS_H_ */
