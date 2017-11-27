/*
 * Copyright (c) 2017 Alain Royer, Jim Tremblay
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

void nOS_InitSpecific(void)
{
}

void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    nOS_Stack *tos = (nOS_Stack*)((uint32_t)(stack + ssize) & 0xFFFFFFF8UL);
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xFFFFFFFFUL;
    }
#endif

    *(--tos) = (nOS_Stack)entry + 4;                        /* PC */
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x00000000;                                  /* LR */
#else
    tos -= 1;
#endif
    *(--tos) = (nOS_Stack)(stack + ssize) & 0xFFFFFFF8UL;   /* SP */
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x12121212UL;                                /* R12 */
    *(--tos) = 0x11111111UL;                                /* R11 */
    *(--tos) = 0x10101010UL;                                /* R10 */
    *(--tos) = 0x09090909UL;                                /* R9 */
    *(--tos) = 0x08080808UL;                                /* R8 */
    *(--tos) = 0x07070707UL;                                /* R7 */
    *(--tos) = 0x06060606UL;                                /* R6 */
    *(--tos) = 0x05050505UL;                                /* R5 */
    *(--tos) = 0x04040404UL;                                /* R4 */
    *(--tos) = 0x03030303UL;                                /* R3 */
    *(--tos) = 0x02020202UL;                                /* R2 */
    *(--tos) = 0x01010101UL;                                /* R1 */
#else
    tos -= 12;                                              /* R12 - R1 */
#endif
    *(--tos) = (nOS_Stack)arg;                              /* R0 */
    *(--tos) = 0x0000001FUL;                                /* SPSR */

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
                nOS_runningThread = nOS_highPrioThread;
            }
        }
#endif
        nOS_LeaveCritical(sr);
    }
}

void SWI_Handler (void)
{
    /* Within an IRQ ISR the link register has an offset from the true return
    address, but an SWI ISR does not.  Add the offset manually so the same
    ISR return code can be used in both cases. */
    __asm volatile ( "ADD   LR, LR, #4" );

    /* Perform the context switch.  First save the context of the current task. */
    PUSH_CONTEXT();

    nOS_runningThread = nOS_highPrioThread;

    /* Restore the context of the new task. */
    POP_CONTEXT();
}

#ifdef __cplusplus
}
#endif
