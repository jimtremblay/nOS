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
#if (__CODE_MODEL__ != __SMALL_CODE_MODEL__)
    /* Function pointers size is 3 bytes */
    *tos-- = (nOS_Stack)((uint32_t)entry);
    *tos-- = (nOS_Stack)((uint32_t)entry >> 8);
    *tos-- = (nOS_Stack)((uint32_t)entry >> 16);
#else
    /* Function pointers size is 2 bytes */
    *tos-- = (nOS_Stack)((uint16_t)entry);
    *tos-- = (nOS_Stack)((uint16_t)entry >> 8);
#endif

    /* Simulate a call of nOS_PushContext */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 'y';                                  /* YL */
    *tos-- = 'Y';                                  /* YH */
#else
     tos -= 2;                                     /* Y */
#endif
#if (__DATA_MODEL__ == __MEDIUM_DATA_MODEL__)
    /* Pointers size is 2 bytes */
    *tos-- = (nOS_Stack)((uint16_t)arg);           /* XL: arg LSB */
    *tos-- = (nOS_Stack)((uint16_t)arg >> 8);      /* XH: arg MSB */
#else
 #if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 'x';                                  /* XL */
    *tos-- = 'X';                                  /* XH */
 #else
     tos -= 2;                                     /* X */
 #endif
#endif
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 'A';                                  /* A */
#else
     tos -= 1;                                     /* A */
#endif
    *tos-- = 0x20;                                 /* CC: Interrupt enabled */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x00;                                 /* ?b0  */
#else
     tos -= 1;                                     /* ?b0  */
#endif
#if (__DATA_MODEL__ == __LARGE_DATA_MODEL__)
    /* Pointers size is 3 bytes */
    *tos-- = (nOS_Stack)((uint32_t)arg >> 16);     /* ?b1 arg MSB */
    *tos-- = (nOS_Stack)((uint32_t)arg >> 8);      /* ?b2   ...   */
    *tos-- = (nOS_Stack)((uint32_t)arg);           /* ?b3 arg LSB */
#else
 #if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x01;                                 /* ?b1  */
    *tos-- = 0x02;                                 /* ?b2  */
    *tos-- = 0x03;                                 /* ?b3  */
 #else
     tos -= 3;                                     /* ?b1 to ?b3 */
 #endif
#endif
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x04;                                 /* ?b4  */
    *tos-- = 0x05;                                 /* ?b5  */
    *tos-- = 0x06;                                 /* ?b6  */
    *tos-- = 0x07;                                 /* ?b7  */
    *tos-- = 0x08;                                 /* ?b8  */
    *tos-- = 0x09;                                 /* ?b9  */
    *tos-- = 0x10;                                 /* ?b10 */
    *tos-- = 0x11;                                 /* ?b11 */
    *tos-- = 0x12;                                 /* ?b12 */
    *tos-- = 0x13;                                 /* ?b13 */
    *tos-- = 0x14;                                 /* ?b14 */
    *tos-- = 0x15;                                 /* ?b15 */
#else
     tos -= 12;                                    /* ?b4 to ?b15 */
#endif

    thread->stackPtr = tos;
}

/* Declare this function as __task; we don't need the compiler to push registers on the stack since we do it manually */
__task void nOS_SwitchContext(void)
{
    PUSH_CONTEXT();
    nOS_runningThread->stackPtr = (nOS_Stack*)__get_cpu_sp();
    nOS_runningThread = nOS_highPrioThread;
#if   (__DATA_MODEL__ == __MEDIUM_DATA_MODEL__)
    __set_cpu_sp((int)nOS_highPrioThread->stackPtr);
#elif (__DATA_MODEL__ == __LARGE_DATA_MODEL__)
    __set_cpu_sp((int)((long)nOS_highPrioThread->stackPtr));
#endif
    POP_CONTEXT();
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
