/*
 * Copyright (c) 2014-2016 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <intrinsics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t                            nOS_Stack;
typedef __ilevel_t                          nOS_StatusReg;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   2
#define NOS_MEM_POINTER_WIDTH               2

#define NOS_16_BITS_SCHEDULER

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined: must be set between 1 and 7 inclusively."
#elif (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO < 1) || (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 7)
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is set to invalid value: must be set between 1 and 7 inclusively."
#else
 #define NOS_MAX_UNSAFE_IPL            NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
#endif

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __get_interrupt_level();                                           \
        if (sr < NOS_MAX_UNSAFE_IPL) {                                          \
            __set_interrupt_level(NOS_MAX_UNSAFE_IPL);                          \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    __set_interrupt_level(sr)

#define nOS_SwitchContext()                                     __asm ("INT #32")

void    nOS_EnterIsr    (void);
bool    nOS_LeaveIsr    (void);

#define NOS_ISR(vect)                                                           \
__task void vect##_ISR_L2(void);                                                \
_Pragma("required=nOS_EnterIsr")                                                \
_Pragma("required=nOS_LeaveIsr")                                                \
_Pragma(_STRINGIFY(vector=##vect))                                              \
__interrupt void vect##_ISR(void)                                               \
{                                                                               \
    __asm (                                                                     \
        /* Push all registers on ISTACK */                                      \
        "PUSHM  R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Increment interrupts nested counter */                               \
        "JSR.A  nOS_EnterIsr                \n"                                 \
                                                                                \
        /* Call user ISR function */                                            \
        "JSR.A  "#vect"_ISR_L2              \n"                                 \
                                                                                \
        /* Ensure interrupts are disabled */                                    \
        "FCLR   I                           \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* Decrement interrupts nested counter */                               \
        /* return true if we need to switch context, otherwise false */         \
        "JSR.A  nOS_LeaveIsr                \n"                                 \
                                                                                \
        /* Do we need to switch context from ISR ? */                           \
        "CMP.B  #0, R0L                     \n"                                 \
        "JEQ    Leave_"#vect"_ISR           \n"                                 \
                                                                                \
        /* YES, we need to switch context */                                    \
                                                                                \
        /* Pop all thread registers */                                          \
        /* nOS_SwitchContextFromIsrHandler will push it all on USTACK */        \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Switch to thread stack */                                            \
        "FSET   U                           \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* Call nOS_SwitchContextFromIsrHandler */                              \
        "INT    #33                         \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* NO, We DON'T need to switch context */                               \
                                                                                \
        "Leave_"#vect"_ISR:                 \n"                                 \
        /* Pop all registers from ISTACK */                                     \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
    );                                                                          \
}                                                                               \
__task void vect##_ISR_L2(void)

/* Unused function */
#define nOS_InitSpecific()

#ifdef NOS_PRIVATE
 void   nOS_InitContext (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
