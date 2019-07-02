/*
 * Copyright (c) 2014-2019 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   8
#define NOS_MEM_POINTER_WIDTH               4

#define NOS_32_BITS_SCHEDULER

#define NOS_SIMULATED_STACK

typedef struct nOS_Stack
{
    HANDLE          handle;
    DWORD           id;
    nOS_ThreadEntry entry;
    void            *arg;
    bool            sync;
    HANDLE          hsync;
} nOS_Stack;

typedef DWORD                               nOS_StatusReg;

#ifndef NOS_CONFIG_TICKS_PER_SECOND
 #error "nOSConfig.h: NOS_CONFIG_TICKS_PER_SECOND is not defined: must be set between 1 and 100 inclusively."
#elif (NOS_CONFIG_TICKS_PER_SECOND < 1) || (NOS_CONFIG_TICKS_PER_SECOND > 100)
 #error "nOSConfig.h: NOS_CONFIG_TICKS_PER_SECOND is set to invalid value: must be set between 1 and 100 inclusively."
#endif

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        NOS_UNUSED(sr);                                                         \
        if (nOS_running) {                                                      \
            /* Enter critical section */                                        \
            while(WaitForSingleObject(nOS_hCritical, INFINITE) != WAIT_OBJECT_0);\
            if (nOS_criticalNestingCounter > 0) {                               \
                /* Keep only one level of lock to leave critical section */     \
                ReleaseMutex(nOS_hCritical);                                    \
            }                                                                   \
            nOS_criticalNestingCounter++;                                       \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    do {                                                                        \
        NOS_UNUSED(sr);                                                         \
        if (nOS_running) {                                                      \
            nOS_criticalNestingCounter--;                                       \
            if (nOS_criticalNestingCounter == 0) {                              \
                /* Leave critical section */                                    \
                ReleaseMutex(nOS_hCritical);                                    \
            }                                                                   \
        }                                                                       \
    } while (0)

#define nOS_PeekCritical()                                                      \
    do {                                                                        \
        uint32_t count = nOS_criticalNestingCounter;                            \
        /* Leave critical section */                                            \
        nOS_criticalNestingCounter = 0;                                         \
        ReleaseMutex(nOS_hCritical);                                            \
                                                                                \
        /* Enter critical section */                                            \
        while(WaitForSingleObject(nOS_hCritical, INFINITE) != WAIT_OBJECT_0);   \
        nOS_criticalNestingCounter = count;                                     \
    } while (0)

extern HANDLE       nOS_hCritical;
extern uint32_t     nOS_criticalNestingCounter;

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
