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

void PendSV_Handler(void) __attribute__( ( naked ) );

#ifdef NOS_CONFIG_ISR_STACK_SIZE
 static nOS_Stack _isrStack[NOS_CONFIG_ISR_STACK_SIZE];
#endif

void nOS_InitSpecific(void)
{
#ifdef NOS_CONFIG_ISR_STACK_SIZE
 #if (NOS_CONFIG_DEBUG > 0)
    size_t i;

    for (i = 0; i < NOS_CONFIG_ISR_STACK_SIZE; i++) {
        _isrStack[i] = 0xFFFFFFFFUL;
    }
 #endif

    /* Copy MSP to PSP */
    _SetPSP(_GetMSP());
    /* Set MSP to local ISR stack */
    _SetMSP((uint32_t)&_isrStack[NOS_CONFIG_ISR_STACK_SIZE] & 0xFFFFFFF8UL);
 #if defined(__VFP_FP__) && !defined(__SOFTFP__)
    /* Set current stack to PSP, privileged mode and FPU active */
    _SetCONTROL(_GetCONTROL() | 0x00000006UL);
 #else
    /* Set current stack to PSP and privileged mode */
    _SetCONTROL(_GetCONTROL() | 0x00000002UL);
 #endif
#else
 #if defined(__VFP_FP__) && !defined(__SOFTFP__)
    /* FPU active */
    _SetCONTROL(_GetCONTROL() | 0x00000004UL);
 #endif
#endif
    /* Set PendSV exception to lowest priority */
    *(volatile uint32_t *)0xE000ED20UL |= 0x00FF0000UL;
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

#if defined(__VFP_FP__) && !defined(__SOFTFP__)
        tos -= 1;
    *(--tos) = 0x00000000UL;    /* FPSCR */
 #if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x15151515UL;    /* S15 */
    *(--tos) = 0x14141414UL;    /* S14 */
    *(--tos) = 0x13131313UL;    /* S13 */
    *(--tos) = 0x12121212UL;    /* S12 */
    *(--tos) = 0x11111111UL;    /* S11 */
    *(--tos) = 0x10101010UL;    /* S10 */
    *(--tos) = 0x09090909UL;    /* S9 */
    *(--tos) = 0x08080808UL;    /* S8 */
    *(--tos) = 0x07070707UL;    /* S7 */
    *(--tos) = 0x06060606UL;    /* S6 */
    *(--tos) = 0x05050505UL;    /* S5 */
    *(--tos) = 0x04040404UL;    /* S4 */
    *(--tos) = 0x03030303UL;    /* S3 */
    *(--tos) = 0x02020202UL;    /* S2 */
    *(--tos) = 0x01010101UL;    /* S1 */
    *(--tos) = 0x00000000UL;    /* S0 */
 #else
        tos -= 16;              /* S15 to S0 */
 #endif
#endif
    *(--tos) = 0x01000000UL;    /* xPSR */
    *(--tos) = (nOS_Stack)entry;/* PC */
    *(--tos) = 0x00000000UL;    /* LR */
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x12121212UL;    /* R12 */
    *(--tos) = 0x03030303UL;    /* R3 */
    *(--tos) = 0x02020202UL;    /* R2 */
    *(--tos) = 0x01010101UL;    /* R1 */
#else
        tos -= 4;               /* R12, R3, R2 and R1 */
#endif
    *(--tos) = (nOS_Stack)arg;  /* R0 */
#if defined(__VFP_FP__) && !defined(__SOFTFP__)
 #if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x31313131UL;    /* S31 */
    *(--tos) = 0x30303030UL;    /* S30 */
    *(--tos) = 0x29292929UL;    /* S29 */
    *(--tos) = 0x28282828UL;    /* S28 */
    *(--tos) = 0x27272727UL;    /* S27 */
    *(--tos) = 0x26262626UL;    /* S26 */
    *(--tos) = 0x25252525UL;    /* S25 */
    *(--tos) = 0x24242424UL;    /* S24 */
    *(--tos) = 0x23232323UL;    /* S23 */
    *(--tos) = 0x22222222UL;    /* S22 */
    *(--tos) = 0x21212121UL;    /* S21 */
    *(--tos) = 0x20202020UL;    /* S20 */
    *(--tos) = 0x19191919UL;    /* S19 */
    *(--tos) = 0x18181818UL;    /* S18 */
    *(--tos) = 0x17171717UL;    /* S17 */
    *(--tos) = 0x16161616UL;    /* S16 */
 #else
        tos -= 16;              /* S31 to S16 */
 #endif
#endif
#if defined(__VFP_FP__) && !defined(__SOFTFP__)
    *(--tos) = 0xFFFFFFEDUL;    /* EXC_RETURN (Thread mode, use FP state from PSP, Thread use PSP */
#else
    *(--tos) = 0xFFFFFFFDUL;    /* EXC_RETURN (Thread mode, don't use FP state, Thread use PSP */
#endif
#if (NOS_CONFIG_DEBUG > 0)
    *(--tos) = 0x11111111UL;    /* R11 */
    *(--tos) = 0x10101010UL;    /* R10 */
    *(--tos) = 0x09090909UL;    /* R9 */
    *(--tos) = 0x08080808UL;    /* R8 */
    *(--tos) = 0x07070707UL;    /* R7 */
    *(--tos) = 0x06060606UL;    /* R6 */
    *(--tos) = 0x05050505UL;    /* R5 */
    *(--tos) = 0x04040404UL;    /* R4 */
#else
        tos -= 8;               /* R11, R10, R9, R8, R7, R6, R5 and R4 */
#endif

    thread->stackPtr = tos;
}

void nOS_EnterIsr (void)
{
    nOS_EnterCritical();
    nOS_isrNestingCounter++;
    nOS_LeaveCritical();
}

void nOS_LeaveIsr (void)
{
    nOS_EnterCritical();
    nOS_isrNestingCounter--;
    if (nOS_isrNestingCounter == 0) {
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
        if (nOS_lockNestingCounter == 0)
#endif
        {
            nOS_highPrioThread = nOS_FindHighPrioThread();
            if (nOS_runningThread != nOS_highPrioThread) {
                *(volatile uint32_t *)0xE000ED04UL = 0x10000000UL;
            }
        }
    }
    nOS_LeaveCritical();
}

void PendSV_Handler(void)
{
    __asm volatile (
        /* Save PSP before doing anything, PendSV_Handler already running on MSP */
        "MRS        R0,         PSP                 \n"
        "ISB                                        \n"

        /* Get the location of nOS_runningThread */
        "LDR        R3,         runningThread       \n"
        "LDR        R2,         [R3]                \n"

#if defined(__VFP_FP__) && !defined(__SOFTFP__)
        /* Push high VFP registers */
        "VSTMDB     R0!,        {S16-S31}           \n"
#endif

        /* Push remaining registers on thread stack */
        "STMDB      R0!,        {R4-R11, LR}        \n"

        /* Save PSP to nOS_Thread object of current running thread */
        "STR        R0,         [R2]                \n"

        /* Get the location of nOS_highPrioThread */
        "LDR        R1,         highPrioThread      \n"
        "LDR        R2,         [R1]                \n"

        /* Copy nOS_highPrioThread to nOS_runningThread */
        "STR        R2,         [R3]                \n"

        /* Restore PSP from nOS_Thread object of high prio thread */
        "LDR        R0,         [R2]                \n"

        /* Pop registers from thread stack */
        "LDMIA      R0!,        {R4-R11, LR}        \n"

#if defined(__VFP_FP__) && !defined(__SOFTFP__)
        /* Pop high VFP registers */
        "VLDMIA     R0!,        {S16-S31}           \n"
#endif

        /* Restore PSP to high prio thread stack */
        "MSR        PSP,        R0                  \n"
        "ISB                                        \n"

        /* Return */
        "BX         LR                              \n"

        ".align 2                                   \n"
        "runningThread: .word nOS_runningThread     \n"
        "highPrioThread: .word nOS_highPrioThread   \n"
    );
}

#ifdef __cplusplus
}
#endif
