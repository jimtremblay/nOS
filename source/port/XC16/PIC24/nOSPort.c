/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if (NOS_CONFIG_ISR_STACK_SIZE > 0)
static nOS_Stack isrStack[NOS_CONFIG_ISR_STACK_SIZE];
#endif

void nOS_ContextInit(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    nOS_Stack *tos = (nOS_Stack*)((uint16_t)stack & 0xFFFE);

    *tos++ = (nOS_Stack)entry;  /* PC LSB */
    *tos++ = 0;                 /* PC MSB */
    *tos++ = 0;                 /* SR */
    *tos++ = (nOS_Stack)arg;    /* W0 */
#if defined(NOS_CONFIG_DEBUG)
    *tos++ = 0x1111;            /* W1 */
    *tos++ = 0x2222;            /* W2 */
    *tos++ = 0x3333;            /* W3 */
    *tos++ = 0x4444;            /* W4 */
    *tos++ = 0x5555;            /* W5 */
    *tos++ = 0x6666;            /* W6 */
    *tos++ = 0x7777;            /* W7 */
    *tos++ = 0x8888;            /* W8 */
    *tos++ = 0x9999;            /* W9 */
    *tos++ = 0xaaaa;            /* W10 */
    *tos++ = 0xbbbb;            /* W11 */
    *tos++ = 0xcccc;            /* W12 */
    *tos++ = 0xdddd;            /* W13 */
    *tos++ = 0xeeee;            /* W14 */
    *tos++ = 0x1234;            /* RCOUNT */
    *tos++ = 0x5678;            /* TBLPAG */
#else
     tos  += 16;                /* W1 to W14, RCOUNT, TBLPAG */
#endif
    *tos++ = CORCON;            /* CORCON */
#if defined(__HAS_EDS__)
    *tos++ = DSRPAG;            /* DSRPAG */
    *tos++ = DSWPAG;            /* DSWPAG */
#else
    *tos++ = PSVPAG;            /* PSVPAG */
#endif

    thread->stackPtr = tos;
}

void __attribute__((naked)) nOS_ContextSwitch(void)
{
    /* Enter critical is not needed here, interrupts are already disabled */
    __asm volatile (
        /* Push all working registers */
        "PUSH   SR                              \n"
        "PUSH.D W0                              \n"
        "PUSH.D W2                              \n"
        "PUSH.D W4                              \n"
        "PUSH.D W6                              \n"
        "PUSH.D W8                              \n"
        "PUSH.D W10                             \n"
        "PUSH.D W12                             \n"
        "PUSH   W14                             \n"
        "PUSH   RCOUNT                          \n"
        "PUSH   TBLPAG                          \n"
        "PUSH   CORCON                          \n"
        PUSH_PAGE_ADDRESS

        /* Get the location of nOS_runningThread */
        "MOV    #_nOS_runningThread,    W0      \n"
        "MOV    [W0],                   W1      \n"

        /* Save SP to nOS_Thread object of current running thread */
        "MOV    W15,                    [W1]    \n"

        /* Get the value of nOS_highPrioThread */
        "MOV    _nOS_highPrioThread,    W2      \n"

        /* Copy nOS_highPrioThread to nOS_runningThread */
        "MOV    W2,                     [W0]    \n"

        /* Restore SP from nOS_Thread object of high prio thread */
        "MOV    [W2],                   W15     \n"

        /* Pop all working registers */
        POP_PAGE_ADDRESS
        "POP    CORCON                          \n"
        "POP    TBLPAG                          \n"
        "POP    RCOUNT                          \n"
        "POP    W14                             \n"
        "POP.D  W12                             \n"
        "POP.D  W10                             \n"
        "POP.D  W8                              \n"
        "POP.D  W6                              \n"
        "POP.D  W4                              \n"
        "POP.D  W2                              \n"
        "POP.D  W0                              \n"
        "POP    SR                              \n"
    );
}

nOS_Stack *nOS_IsrEnter (nOS_Stack *sp)
{
    // Enter critical here is not needed, interrupts are already disabled
    if (nOS_isrNestingCounter == 0) {
        nOS_runningThread->stackPtr = sp;
#if (NOS_CONFIG_ISR_STACK_SIZE > 0)
        sp = &isrStack[0];
#else
        sp = nOS_idleHandle.stackPtr;
#endif
    }
    nOS_isrNestingCounter++;

    return sp;
}

nOS_Stack *nOS_IsrLeave (nOS_Stack *sp)
{
    // Enter critical here is not needed, interrupts are already disabled
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
            nOS_highPrioThread = SchedHighPrio();
#else
            nOS_highPrioThread = nOS_ListHead(&nOS_readyList);
#endif
            nOS_runningThread = nOS_highPrioThread;
            sp = nOS_runningThread->stackPtr;
        }
    }

    return sp;
}

#if defined(__cplusplus)
}
#endif
