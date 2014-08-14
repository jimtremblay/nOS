/*
 * Copyright (c) 2014 Jim Tremblay
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

#if defined(NOS_CONFIG_ISR_STACK_SIZE)
 #if (NOS_CONFIG_ISR_STACK_SIZE == 0)
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #else
  static nOS_Stack isrStack[NOS_CONFIG_ISR_STACK_SIZE];
 #endif
#endif

void nOS_ContextInit(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, void(*func)(void*), void *arg)
{
    /* Stack grow from high to low address */
    nOS_Stack   *tos    = stack + (ssize - 1);

    *tos-- = (uint16_t)(func);
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

void nOS_ContextSwitch(void)
{
    asm volatile (
        "push.w     sr                                      \n"
        "                                                   \n"
        /* Push all registers to running thread stack */
        "pushm.w    #12,                r15                 \n"
        "                                                   \n"
        /* Save stack pointer to running thread structure */
        "mov.w      &nOS_runningThread, r12                 \n"
        "mov.w      sp,                 0(r12)              \n"
        "                                                   \n"
        "mov.w      nOS_highPrioThread, nOS_runningThread   \n"
        "                                                   \n"
        /* Restore stack pointer from high prio thread structure */
        "mov.w      &nOS_runningThread, r12                 \n"
        "mov.w      @r12,               sp                  \n"
        "                                                   \n"
        /* Pop all registers from high prio thread stack */
        "popm.w     #12,                r15                 \n"
        "                                                   \n"
        "pop.w      sr                                      \n"
        "                                                   \n"
        "ret                                                \n"
    );
}

nOS_Stack* nOS_IsrEnter (nOS_Stack *sp)
{
    if (nOS_isrNestingCounter == 0) {
        nOS_runningThread->stackPtr = sp;
#if defined(NOS_CONFIG_ISR_STACK_SIZE)
        sp = &isrStack[NOS_CONFIG_ISR_STACK_SIZE-1];
#else
        sp = nOS_mainThread.stackPtr;
#endif
    }
    nOS_isrNestingCounter++;

    return sp;
}

nOS_Stack* nOS_IsrLeave (nOS_Stack *sp)
{
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
            nOS_highPrioThread = SchedHighPrio();
            nOS_runningThread = nOS_highPrioThread;
        }
        sp = nOS_runningThread->stackPtr;
    }

    return sp;
}

#if defined(__cplusplus)
}
#endif
