/*
 * Copyright (c) 2014-2015 Jim Tremblay
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

void nOS_InitContext(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    /* Stack grow from high to low address */
    nOS_Stack   *tos    = stack + (ssize - 1);
#if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < ssize; i++) {
        stack[i] = 0xFFFF;
    }
#endif

    *tos-- = (uint16_t)entry;           /* PC */
    *tos-- = 0x0008;                    /* Interrupts enabled */

#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x1515;                    /* R15 */
    *tos-- = 0x1414;                    /* R14 */
    *tos-- = 0x1313;                    /* R13 */
#else
     tos  -= 3;                         /* R15 to R13 */
#endif
    *tos-- = (nOS_Stack)arg;            /* R12 */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = 0x1111;                    /* R11 */
    *tos-- = 0x1010;                    /* R10 */
    *tos-- = 0x0909;                    /* R9 */
    *tos-- = 0x0808;                    /* R8 */
    *tos-- = 0x0707;                    /* R7 */
    *tos-- = 0x0606;                    /* R6 */
    *tos-- = 0x0505;                    /* R5 */
    *tos   = 0x0404;                    /* R4 */
#else
     tos  -= 7;                         /* R11 to R4 */
#endif

    thread->stackPtr = tos;
}

void nOS_SwitchContext(void)
{
    SAVE_CONTEXT();
    nOS_runningThread = nOS_highPrioThread;
    RESTORE_CONTEXT();
    asm volatile ("ret");
}

nOS_Stack* nOS_EnterIsr (nOS_Stack *sp)
{
    if (nOS_isrNestingCounter == 0) {
        nOS_runningThread->stackPtr = sp;
#ifdef NOS_CONFIG_ISR_STACK_SIZE
        sp = &_isrStack[NOS_CONFIG_ISR_STACK_SIZE-1];
#else
        sp = nOS_idleHandle.stackPtr;
#endif
    }
    nOS_isrNestingCounter++;

    return sp;
}

nOS_Stack* nOS_LeaveIsr (nOS_Stack *sp)
{
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
            nOS_highPrioThread = nOS_FindHighPrioThread();
            nOS_runningThread = nOS_highPrioThread;
        }
        sp = nOS_runningThread->stackPtr;
    }

    return sp;
}

#ifdef __cplusplus
}
#endif
