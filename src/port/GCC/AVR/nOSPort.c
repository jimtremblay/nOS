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

#ifdef NOS_CONFIG_ISR_STACK_SIZE
 static nOS_Stack _isrStack[NOS_CONFIG_ISR_STACK_SIZE];
#endif

void nOS_InitSpecific(void)
{
#ifdef NOS_CONFIG_ISR_STACK_SIZE
 #if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < NOS_CONFIG_ISR_STACK_SIZE; i++) {
        _isrStack[i] = 0xFF;
    }
 #endif
#endif
}

void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    /* Stack grow from high to low address */
    nOS_Stack *tos = stack + (ssize - 1);
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xFF;
    }
#endif

    /* Simulate a call to thread function */
    *tos-- = (nOS_Stack)((uint16_t)entry);
    *tos-- = (nOS_Stack)((uint16_t)entry >> 8);
#ifdef __AVR_3_BYTE_PC__
    *tos-- = 0x00;                                  /* Always set high part of address to 0 */
#endif

    /* Simulate a call of nOS_PushContext */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x01;                                  /* R0 */
#else
     tos  -= 1;                                     /* R0 */
#endif
    *tos-- = 0x80;                                  /* SREG: Interrupts enabled */
#ifdef __AVR_HAVE_RAMPZ__
     tos  -= 1;                                     /* RAMPZ */
#endif
#ifdef __AVR_3_BYTE_PC__
     tos  -= 1;                                     /* EIND */
#endif
    *tos-- = 0x00;                                  /* R1 always 0 */
#if (NOS_CONFIG_DEBUG > 0)
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
    *tos-- = 0x16;                                  /* R16 */
    *tos-- = 0x17;                                  /* R17 */
    *tos-- = 0x18;                                  /* R18 */
    *tos-- = 0x19;                                  /* R19 */
    *tos-- = 0x20;                                  /* R20 */
    *tos-- = 0x21;                                  /* R21 */
    *tos-- = 0x22;                                  /* R22 */
    *tos-- = 0x23;                                  /* R23 */
#else
     tos  -= 22;                                    /* R2 to R23 */
#endif
    *tos-- = (nOS_Stack)((uint16_t)arg);            /* R24: arg LSB */
    *tos-- = (nOS_Stack)((uint16_t)arg >> 8);       /* R25: arg MSB */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x26;                                  /* R26 */
    *tos-- = 0x27;                                  /* R27 */
    *tos-- = 0x28;                                  /* R28 */
    *tos-- = 0x29;                                  /* R29 */
    *tos-- = 0x30;                                  /* R30 */
    *tos-- = 0x31;                                  /* R31 */
#else
     tos  -= 6;                                     /* R26 to R31 */
#endif

    thread->stackPtr = tos;
}

/* Absolutely need a naked function because function call push the return address on the stack */
void nOS_SwitchContext(void)
{
    PUSH_CONTEXT();
    nOS_runningThread->stackPtr = (uint8_t*)SP;
    nOS_runningThread = nOS_highPrioThread;
    SP = (int)nOS_highPrioThread->stackPtr;
    POP_CONTEXT();
    asm volatile("ret");
}

nOS_Stack *nOS_EnterIsr (nOS_Stack *sp)
{
#if (NOS_CONFIG_SAFE > 0)
    if (nOS_running)
#endif
    {
        /* Interrupts already disabled when entering in ISR */
        if (nOS_isrNestingCounter == 0) {
            nOS_runningThread->stackPtr = sp;
#ifdef NOS_CONFIG_ISR_STACK_SIZE
            sp = &_isrStack[NOS_CONFIG_ISR_STACK_SIZE-1];
#else
            sp = nOS_idleHandle.stackPtr;
#endif
        }
        nOS_isrNestingCounter++;
    }

    return sp;
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
