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

static nOS_Stack _isrStack[NOS_CONFIG_ISR_STACK_SIZE];

void nOS_InitSpecific(void)
{
    nOS_Stack  *sp = &_isrStack[NOS_CONFIG_ISR_STACK_SIZE-1];
    uint32_t    reg;
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < NOS_CONFIG_ISR_STACK_SIZE; i++) {
        _isrStack[i] = 0xFFFFFFFFUL;
    }
#endif

    __asm volatile (
        /* Set USP to current value of ISP */
        "MVFC   ISP,    %[reg]  \n"
        "MVTC   %[reg], USP     \n"
        /* Set ISP to local ISR stack */
        "MVTC   %[sp],  ISP     \n"
        /* Set current stack used to USP */
        "SETPSW U               \n"
        : [reg] "=r" (reg)
        : [reg] "r" (reg), [sp] "r" ((uint32_t)sp)
    );

    /* Enable software interrupt */
    *(uint8_t*)0x00087203UL |= (uint8_t)0x08;
    /* Clear software interrupt */
    *(uint8_t*)0x00087003UL &=~ (uint8_t)0x08;
    /* Set software interrupt to lowest priority level */
    *(uint8_t*)0x00087303UL = (uint8_t)1;
}

void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    nOS_Stack *tos = (stack + (ssize - 1));
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xFFFFFFFFUL;
    }
#endif

    *tos-- = 0x00030000UL;      /* Interrupts enabled, User stack selected, Supervisor mode */
    *tos-- = (nOS_Stack)entry;
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x15151515UL;
    *tos-- = 0x14141414UL;
    *tos-- = 0x13131313UL;
    *tos-- = 0x12121212UL;
    *tos-- = 0x11111111UL;
    *tos-- = 0x10101010UL;
    *tos-- = 0x09090909UL;
    *tos-- = 0x08080808UL;
    *tos-- = 0x07070707UL;
    *tos-- = 0x06060606UL;
    *tos-- = 0x05050505UL;
    *tos-- = 0x04040404UL;
    *tos-- = 0x03030303UL;
    *tos-- = 0x02020202UL;
#else
     tos  -= 14;
#endif
    *tos-- = (nOS_Stack)arg;
    *tos-- = 0x00000100UL;      /* Floating-point status word (default) */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x01234567UL;      /* Accumulator high */
    *tos   = 0x89abcdefUL;      /* Accumulator low */
#else
     tos  -= 1;
#endif

    thread->stackPtr = tos;
}

void nOS_EnterIsr (void)
{
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (nOS_running)
#endif
    {
        nOS_EnterCritical(sr);
        nOS_isrNestingCounter++;
        nOS_LeaveCritical(sr);
    }
}

void nOS_LeaveIsr (void)
{
    nOS_StatusReg   sr;

#if (NOS_CONFIG_SAFE > 0)
    if (nOS_running)
#endif
    {
        nOS_EnterCritical(sr);
        nOS_isrNestingCounter--;
#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0) || (NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE > 0)
        if (nOS_isrNestingCounter == 0) {
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
                if (nOS_runningThread != nOS_highPrioThread) {
                    /* Request a software interrupt when going out of ISR */
                    *(uint8_t*)0x000872E0UL = (uint8_t)1;
                }
            }
        }
#endif
        nOS_LeaveCritical(sr);
    }
}

#ifdef __cplusplus
}
#endif
