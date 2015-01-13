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

#if defined(NOS_CONFIG_ISR_STACK_SIZE)
 #if (NOS_CONFIG_ISR_STACK_SIZE == 0)
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #else
  static nOS_Stack isrStack[NOS_CONFIG_ISR_STACK_SIZE];
 #endif
#endif

void nOS_ContextInit(nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg)
{
    /* Stack grow from high to low address */
    nOS_Stack   *tos    = stack + (ssize - 1);
    uint16_t    *tos16;

#if (NOS_CONFIG_DEBUG > 0)
    /* Place some fences around thread's stack to know if an overflow occurred */
    *stack++ = (nOS_Stack)0x01234567;
    *stack   = (nOS_Stack)0x89abcdef;
    *tos--   = (nOS_Stack)0x76543210;
    *tos--   = (nOS_Stack)0xfedcba98;
#endif

     tos16   = (uint16_t*)tos;
#if (__CORE__ == __430X__)
    *tos16-- = (uint16_t)((uint32_t)entry >> 16);   /* PC MSB */
#endif
    *tos16-- = (uint16_t)((uint32_t)entry);         /* PC LSB */
    *tos16-- = 0x0008;                              /* SR */
#if (__CORE__ == __430X__) && ((__DATA_MODEL__ == __DATA_MODEL_MEDIUM__) || (__DATA_MODEL__ == __DATA_MODEL_LARGE__))
     tos16  -= 1;
#endif
     tos     = (nOS_Stack*)tos16;

#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = (nOS_Stack)0x15151515;     /* R15 */
    *tos-- = (nOS_Stack)0x14141414;     /* R14 */
    *tos-- = (nOS_Stack)0x13131313;     /* R13 */
#else
     tos  -= 3;                         /* R15 to R13 */
#endif
    *tos-- = (nOS_Stack)arg;            /* R12 */
#if (NOS_CONFIG_DEBUG > 0)
    *tos-- = (nOS_Stack)0x11111111;     /* R11 */
    *tos-- = (nOS_Stack)0x10101010;     /* R10 */
    *tos-- = (nOS_Stack)0x09090909;     /* R9 */
    *tos-- = (nOS_Stack)0x08080808;     /* R8 */
    *tos-- = (nOS_Stack)0x07070707;     /* R7 */
    *tos-- = (nOS_Stack)0x06060606;     /* R6 */
    *tos-- = (nOS_Stack)0x05050505;     /* R5 */
    *tos   = (nOS_Stack)0x04040404;     /* R4 */
#else
     tos  -= 7;                         /* R11 to R4 */
#endif

    thread->stackPtr = tos;
}

__task void nOS_ContextSwitch(void)
{
    __asm (
        /* Simulate an interrupt by pushing SR */
         PUSH_SR
        "                                           \n"
        /* Save all registers to running thread stack */
         PUSH_CONTEXT
        "                                           \n"
        /* Save stack pointer to running thread structure */
         MOV_X"     &nOS_runningThread,     R12     \n"
         MOV_X"     SP,                     0(R12)  \n"
        "                                           \n"
        /* Copy nOS_highPrioThread to nOS_runningThread */
         MOV_X"     &nOS_highPrioThread,    R12     \n"
         MOV_X"     #nOS_runningThread,     R11     \n"
         MOV_X"     R12,                    0(R11)  \n"
        "                                           \n"
        /* Restore stack pointer from high prio thread structure */
         MOV_X"     @R12,                   SP      \n"
        "                                           \n"
        /* Pop all registers from high prio thread stack */
         POP_CONTEXT
        "                                           \n"
         POP_SR
         RET_X
    );
}

nOS_Stack* nOS_IsrEnter (nOS_Stack *sp)
{
    nOS_CriticalEnter();
    if (nOS_isrNestingCounter == 0) {
        nOS_runningThread->stackPtr = sp;
#if defined(NOS_CONFIG_ISR_STACK_SIZE)
        sp = &isrStack[NOS_CONFIG_ISR_STACK_SIZE-1];
#else
        sp = nOS_mainHandle.stackPtr;
#endif
    }
    nOS_isrNestingCounter++;
    nOS_CriticalLeave();

    return sp;
}

nOS_Stack* nOS_IsrLeave (nOS_Stack *sp)
{
    nOS_CriticalEnter();
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
    nOS_CriticalLeave();

    return sp;
}

#if defined(__cplusplus)
}
#endif
