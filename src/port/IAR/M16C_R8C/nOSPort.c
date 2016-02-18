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

void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    nOS_Stack *tos = (nOS_Stack*)((uint16_t)(stack + ssize) & 0xFFFE);
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xFFFF;
    }
#endif

    /* FLG = 0x0C0 = interrupts enabled, USP stack pointer */
    *(--tos) = (((uint32_t)entry >> 8) & 0x0F00) | 0x00C0;
    *(--tos) = (nOS_Stack)((uint32_t)entry);

#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0xFFFF;          /* FB */
    *(--tos) = 0xCCCC;          /* SB */
    *(--tos) = 0xBBBB;          /* A1 */
#else
        tos -= 3;               /* FB, SB, A1 */
#endif
    *(--tos) = (nOS_Stack)arg;  /* A0 */
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x4444;          /* R3 */
    *(--tos) = 0x3333;          /* R2 */
    *(--tos) = 0x2222;          /* R1 */
    *(--tos) = 0x1111;          /* R0 */
#else
        tos -= 4;               /* R3, R2, R1, R0 */
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

bool nOS_LeaveIsr (void)
{
    nOS_StatusReg   sr;
    bool            swctx = false;

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
                    swctx = true;
                }
            }
        }
#endif
        nOS_LeaveCritical(sr);
    }

    return swctx;
}

#pragma vector = 0x20
__interrupt void nOS_SwitchContextHandler(void)
{
    __asm (
        /* Push all registers on thread stack */
        "PUSHM  R0,R1,R2,R3,A0,A1,SB,FB                     \n"

        /* Save SP to nOS_Thread object of current running thread */
        "MOV.W  nOS_runningThread,      A0                  \n"
        "STC    SP,                     [A0]                \n"

        /* Copy nOS_highPrioThread to nOS_runningThread */
        "MOV.W  nOS_highPrioThread,     nOS_runningThread   \n"

        /* Restore SP from nOS_Thread object of high prio thread */
        "MOV.W  nOS_highPrioThread,     A0                  \n"
        "LDC    [A0],                   SP                  \n"

        /* Pop all registers from thread stack */
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB                     \n"
    );
}

#pragma vector = 0x21
__interrupt void nOS_SwitchContextFromIsrHandler(void)
{
    __asm (
        /* Push all registers on thread stack */
        "PUSHM  R0,R1,R2,R3,A0,A1,SB,FB                     \n"

        /* Move PC and FLG from ISTACK to USTACK */
        "STC    ISP,                    A0                  \n"
        "MOV.W  0[A0],                  16[SP]              \n"
        "MOV.W  2[A0],                  18[SP]              \n"

        /* Adjust ISTACK (remove PC and FLG) */
        "ADD.W  #4,                     A0                  \n"
        "LDC    A0,                     ISP                 \n"

        /* Save SP to nOS_Thread object of current running thread */
        "MOV.W  nOS_runningThread,      A0                  \n"
        "STC    SP,                     [A0]                \n"

        /* Copy nOS_highPrioThread to nOS_runningThread */
        "MOV.W  nOS_highPrioThread,     nOS_runningThread  \n"

        /* Restore SP from nOS_Thread object of high prio thread */
        "MOV.W  nOS_highPrioThread,     A0                  \n"
        "LDC    [A0],                   SP                  \n"

        /* Pop all registers from thread stack */
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB                     \n"
    );
}

#ifdef __cplusplus
}
#endif
