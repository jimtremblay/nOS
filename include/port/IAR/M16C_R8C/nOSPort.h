/*
 * Copyright (c) 2014-2015 Jim Tremblay
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

#define NOS_UNUSED(v)                       (void)v

/* Not needed to align memory blocks */
#define NOS_PORT_MEM_ALIGNMENT              1

#define NOS_PORT_SCHED_USE_16_BITS

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined: must be set between 1 and 7 inclusively."
#elif (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO < 1) || (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 7)
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is set to invalid value: must be set between 1 and 7 inclusively."
#else
 #define NOS_PORT_MAX_UNSAFE_IPL            NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
#endif

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    __ilevel_t _ipl = __get_interrupt_level();                                  \
    if (_ipl < NOS_PORT_MAX_UNSAFE_IPL) {                                       \
        __set_interrupt_level(NOS_PORT_MAX_UNSAFE_IPL);                         \
    }

#define nOS_CriticalLeave()                                                     \
    __set_interrupt_level(_ipl);                                                \
}

#define nOS_ContextSwitch()                                     __asm ("INT #32")

void    nOS_IsrEnter    (void);
bool    nOS_IsrLeave    (void);

#define NOS_ISR(vect)                                                           \
__task void vect##_ISR_L2(void);                                                \
_Pragma("required=nOS_IsrEnter")                                                \
_Pragma("required=nOS_IsrLeave")                                                \
_Pragma(_STRINGIFY(vector=##vect))                                              \
__interrupt void vect##_ISR(void)                                               \
{                                                                               \
    __asm (                                                                     \
        /* Push all registers on ISTACK */                                      \
        "PUSHM  R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Increment interrupts nested counter */                               \
        "JSR.A  nOS_IsrEnter                \n"                                 \
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
        "JSR.A  nOS_IsrLeave                \n"                                 \
                                                                                \
        /* Do we need to switch context from ISR ? */                           \
        "CMP.B  #0, R0L                     \n"                                 \
        "JEQ    Leave_"#vect"_ISR           \n"                                 \
                                                                                \
        /* YES, we need to switch context */                                    \
                                                                                \
        /* Pop all thread registers */                                          \
        /* nOS_PortContextSwitchFromIsr will push it all on USTACK */           \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Switch to thread stack */                                            \
        "FSET   U                           \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* Call nOS_PortContextSwitchFromIsr */                                 \
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
#define nOS_PortInit()

void    nOS_ContextInit                 (nOS_Thread *thread, nOS_Stack *stack, size_t ssize,
                                         nOS_ThreadEntry entry, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
