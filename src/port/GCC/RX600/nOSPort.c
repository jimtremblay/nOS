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

void INT_Excep_ICU_SWINT(void) __attribute__((naked));

void nOS_InitSpecific(void)
{
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
    *tos   = 0x89ABCDEFUL;      /* Accumulator low */
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

void INT_Excep_ICU_SWINT(void)
{
    __asm volatile(
        /* Push R15 on ISP (we will use it) */
        "PUSH.L     R15                             \n"

        /* Get running thread stack and adjust it to contains PSW, PC and R15 */
        "MVFC       USP,                    R15     \n"
        "SUB        #12,                    R15     \n"

        /* Set USP to adjusted value */
        "MVTC       R15,                    USP     \n"

        /* Moved pushed registers from ISP to running thread stack */
        "MOV.L      [R0],                   [R15]   \n"
        "MOV.L     4[R0],                  4[R15]   \n"
        "MOV.L     8[R0],                  8[R15]   \n"

        /* Adjust ISP (Remove R15, PC and PSW from the stack) */
        "ADD        #12,                    R0      \n"

        /* At this point, we can continue on USP */
        "SETPSW     U                               \n"

        /* Push all remaining registers to running thread stack */
        "PUSHM      R1-R14                          \n"

        /* Push floating-point status register to running thread stack */
        "PUSHC      FPSW                            \n"

        /* Push accumulator register to running thread stack */
        "MVFACHI    R15                             \n"
        "MVFACMI    R14                             \n"
        "SHLL       #16,                    R14     \n"
        "PUSHM      R14-R15                         \n"

        /* Save SP in nOS_runningThread object */
        "MOV.L      #_nOS_runningThread,    R15     \n"
        "MOV.L      [R15],                  R14     \n"
        "MOV.L      R0,                     [R14]   \n"

        /* nOS_runningThread = nOS_highPrioThread */
        "MOV.L      #_nOS_highPrioThread,   R14     \n"
        "MOV.L      [R14],                  [R15]   \n"

        /* Restore SP from nOS_highPrioThread object */
        "MOV.L      [R14],                  R15     \n"
        "MOV.L      [R15],                  R0      \n"

        /* Pop accumulator register from high prio thread stack */
        "POPM       R14-R15                         \n"
        "MVTACLO    R14                             \n"
        "MVTACHI    R15                             \n"

        /* Pop floating-point status register from high prio thread stack */
        "POPC       FPSW                            \n"

        /* Pop all registers from high prio thread stack */
        "POPM       R1-R15                          \n"

        /* Return from interrupt (will pop PC and PSW) */
        "RTE                                        \n"
        "NOP                                        \n"
        "NOP                                        \n"
    );
}

#ifdef __cplusplus
}
#endif
