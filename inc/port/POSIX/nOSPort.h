/*
 * Copyright (c) 2014-2019 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   8
#ifdef __x86_64__
 #define NOS_MEM_POINTER_WIDTH              8
#else
 #define NOS_MEM_POINTER_WIDTH              4
#endif

#define NOS_32_BITS_SCHEDULER

#define NOS_SIMULATED_STACK

typedef struct nOS_Stack
{
    pthread_t           handle;
    nOS_ThreadEntry     entry;
    void                *arg;
    uint32_t            crit;
    pthread_cond_t      cond;
    bool                started;
    bool                running;
} nOS_Stack;

typedef uint32_t                            nOS_StatusReg;

#ifndef NOS_CONFIG_TICKS_PER_SECOND
 #error "nOSConfig.h: NOS_CONFIG_TICKS_PER_SECOND is not defined: must be set between 1 and 100 inclusively."
#elif (NOS_CONFIG_TICKS_PER_SECOND < 1) || (NOS_CONFIG_TICKS_PER_SECOND > 100)
 #error "nOSConfig.h: NOS_CONFIG_TICKS_PER_SECOND is set to invalid value: must be set between 1 and 100 inclusively."
#endif

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        NOS_UNUSED(sr);                                                         \
        pthread_mutex_lock(&nOS_criticalSection);                               \
        if (nOS_criticalNestingCounter > 0) {                                   \
            /* Lock mutex only one time */                                      \
            pthread_mutex_unlock(&nOS_criticalSection);                         \
        }                                                                       \
        nOS_criticalNestingCounter++;                                           \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    do {                                                                        \
        NOS_UNUSED(sr);                                                         \
        nOS_criticalNestingCounter--;                                           \
        if (nOS_criticalNestingCounter == 0) {                                  \
            /* Unlock mutex when nesting counter reach zero */                  \
            pthread_mutex_unlock(&nOS_criticalSection);                         \
        }                                                                       \
    } while (0)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        uint32_t count = nOS_criticalNestingCounter;                            \
        /* Leave critical section */                                            \
        nOS_criticalNestingCounter = 0;                                         \
        pthread_mutex_unlock(&nOS_criticalSection);                             \
                                                                                \
        /* Enter critical section */                                            \
        pthread_mutex_lock(&nOS_criticalSection);                               \
        nOS_criticalNestingCounter = count;                                     \
    } while (0)

extern volatile uint32_t    nOS_criticalNestingCounter;
extern pthread_mutex_t      nOS_criticalSection;

int     nOS_Print           (const char *format, ...);

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific    (void);
 void   nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
 void   nOS_SwitchContext   (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
