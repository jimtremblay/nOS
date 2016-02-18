/*
 * Copyright (c) 2014-2016 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#ifdef __cplusplus
extern "C" {
#endif

void nOS_InitContext (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, size_t cssize, nOS_ThreadEntry entry, void *arg)
{
    /* Stack grow from high to low address */
    nOS_Stack *tocs = stack + (ssize - 1);
    nOS_Stack *tos;
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xFF;
    }
#endif

#if defined(__ATmega2560__) || defined(__ATmega2561__)
     tos     = tocs - (cssize * 3);
#else
     tos     = tocs - (cssize * 2);
#endif

    /* Simulate a call to thread function */
#if defined(__ATmega2560__) || defined(__ATmega2561__)
    *tocs--  = (nOS_Stack)((uint32_t)entry);
    *tocs--  = (nOS_Stack)((uint32_t)entry >> 8);
    *tocs--  = (nOS_Stack)((uint32_t)entry >> 16);
#else
    *tocs--  = (nOS_Stack)((uint16_t)entry);
    *tocs--  = (nOS_Stack)((uint16_t)entry >> 8);
#endif

    /* Simulate a call of nOS_PushContext */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x00;                                  /* R0 */
#else
     tos  -= 1;                                     /* R0 */
#endif
    *tos-- = 0x80;                                  /* SREG: Interrupts enabled */
    *tos-- = (nOS_Stack)((uint16_t)tocs);           /* R28 */
    *tos-- = (nOS_Stack)((uint16_t)tocs >> 8);      /* R29 */
#ifdef __HAS_RAMPZ__
     tos  -= 1;                                     /* RAMPZ */
#endif
#if defined(__ATmega2560__) || defined(__ATmega2561__)
     tos  -= 1;                                     /* EIND */
#endif
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x01;                                  /* R1 */
    *tos-- = 0x02;                                  /* R2 */
    *tos-- = 0x03;                                  /* R3 */
    *tos-- = 0x04;                                  /* R4 */
    *tos-- = 0x05;                                  /* R5 */
    *tos-- = 0x06;                                  /* R6 */
    *tos-- = 0x07;                                  /* R7 */
    *tos-- = 0x08;                                  /* R8 */
    *tos-- = 0x09;                                  /* R9 */
    *tos-- = 0x10;                                  /* R10 */
    *tos-- = 0x11;                                  /* R11 */
    *tos-- = 0x12;                                  /* R12 */
    *tos-- = 0x13;                                  /* R13 */
    *tos-- = 0x14;                                  /* R14 */
    *tos-- = 0x15;                                  /* R15 */
#else
     tos  -= 15;                                    /* R1 to R15 */
#endif
    *tos-- = (nOS_Stack)((uint16_t)arg);            /* R16: arg LSB */
    *tos-- = (nOS_Stack)((uint16_t)arg >> 8);       /* R17: arg MSB */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x18;                                  /* R18 */
    *tos-- = 0x19;                                  /* R19 */
    *tos-- = 0x20;                                  /* R20 */
    *tos-- = 0x21;                                  /* R21 */
    *tos-- = 0x22;                                  /* R22 */
    *tos-- = 0x23;                                  /* R23 */
    *tos-- = 0x24;                                  /* R24 */
    *tos-- = 0x25;                                  /* R25 */
    *tos-- = 0x26;                                  /* R26 */
    *tos-- = 0x27;                                  /* R27 */
    *tos-- = 0x30;                                  /* R30 */
    *tos   = 0x31;                                  /* R31 */
#else
     tos  -= 11;                                    /* R18 to R27, R30, R31 */
#endif

    thread->stackPtr = tos;
}

/* Absolutely need a naked function because function call push the return address on the stack */
__task void nOS_SwitchContext (void)
{
    PUSH_CONTEXT();
    __asm (
        "lds    r26,    nOS_runningThread           \n"
        "lds    r27,    nOS_runningThread + 1       \n"
        "st     x+,     r28                         \n"
        "st     x+,     r29                         \n"
    );
    nOS_runningThread = nOS_highPrioThread;
    __asm (
        "lds    r26,    nOS_highPrioThread          \n"
        "lds    r27,    nOS_highPrioThread + 1      \n"
        "ld     r28,    x+                          \n"
        "ld     r29,    x+                          \n"
    );
    POP_CONTEXT();
}

void nOS_EnterIsr (nOS_Stack *sp)
{
#if (NOS_CONFIG_SAFE > 0)
    if (nOS_running)
#endif
    {
        /* Interrupts already disabled when entering in ISR */
        if (nOS_isrNestingCounter == 0) {
            nOS_runningThread->stackPtr = sp;
        }
        nOS_isrNestingCounter++;
    }
}

nOS_Stack *nOS_LeaveIsr (nOS_Stack *sp)
{
#if (NOS_CONFIG_SAFE > 0)
    if (nOS_running)
#endif
    {
        /* Interrupts already disabled before leaving ISR */
        nOS_isrNestingCounter--;
        if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0) || (NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE > 0)
 #if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
            if (nOS_lockNestingCounter == 0)
 #endif
            {
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO == 0)
                nOS_highPrioThread = nOS_GetHeadOfList(&nOS_readyThreadsList);
 #elif (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
                nOS_highPrioThread = nOS_FindHighPrioThread();
 #else
                nOS_highPrioThread = nOS_GetHeadOfList(&nOS_readyThreadsList[nOS_runningThread->prio]);
 #endif
                nOS_runningThread = nOS_highPrioThread;
            }
#endif
            sp = nOS_runningThread->stackPtr;
        }
    }

    return sp;
}

#ifdef __cplusplus
}
#endif
