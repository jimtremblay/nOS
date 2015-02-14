/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint16_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

/* Not needed to align memory blocks */
#define NOS_PORT_MEM_ALIGNMENT              1

#define NOS_PORT_SCHED_USE_16_BITS
#define NOS_PORT_NO_CONST

#if !defined(NOS_CONFIG_MAX_UNSAFE_ISR_PRIO)
 #define NOS_CONFIG_MAX_UNSAFE_ISR_PRIO     3
 #warning "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined (default to 3)."
#endif

#define NOS_PORT_MAX_UNSAFE_IPL             NOS_CONFIG_MAX_UNSAFE_ISR_PRIO

__attribute__( ( always_inline ) ) static inline uint16_t GetIPL(void)
{
    uint16_t ipl;
    __asm volatile(
        "STC    FLG,        %0      \n"
        "AND.W  #0x7000,    %0      \n"
        "SHL.W  #-8,        %0      \n"
        "SHL.W  #-4,        %0      \n"
    : "=r" (ipl));
    return ipl;
}

__attribute__( ( always_inline ) ) static inline void SetIPL(uint16_t ipl)
{
    uint16_t flg;
    __asm volatile (
        "STC    FLG,        %0      \n"
        "AND.W  #0x8FFF,    %0      \n"
        "SHL.W  #8,         %1      \n"
        "SHL.W  #4,         %1      \n"
        "OR.W   %1,         %0      \n"
        "LDC    %0,         FLG     \n"
    :: "r" (flg), "r" (ipl));
}

__attribute( ( always_inline ) ) static inline void EnableInterrupt(void)
{
    __asm volatile("FSET I");
}

__attribute( ( always_inline ) ) static inline void DisableInterrupt(void)
{
    __asm volatile("FCLR I");
}

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    uint16_t _ipl = GetIPL();                                                   \
    if (_ipl < NOS_PORT_MAX_UNSAFE_IPL) {                                       \
        __asm volatile (                                                        \
            "LDIPL  %0              \n"                                         \
        :: "i" (NOS_PORT_MAX_UNSAFE_IPL));                                      \
    }


#define nOS_CriticalLeave()                                                     \
    SetIPL(_ipl);                                                               \
}

#define nOS_ContextSwitch()                             __asm volatile("INT #32")

void    nOS_IsrEnter    (void);
bool    nOS_IsrLeave    (void);

#define NOS_ISR(func)                                                           \
void func(void) __attribute__ ((interrupt));                                    \
void func##_L2(void);                                                           \
void func(void)                                                                 \
{                                                                               \
    __asm volatile (                                                            \
        /* Push all registers on ISTACK */                                      \
        "PUSHM  R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Increment interrupts nested counter */                               \
        "JSR.A  _nOS_IsrEnter               \n"                                 \
                                                                                \
        /* Call user ISR function */                                            \
        "JSR.A  _"#func"_L2                 \n"                                 \
                                                                                \
        /* Ensure interrupts are disabled */                                    \
        "FCLR   I                           \n"                                 \
                                                                                \
        /* Decrement interrupts nested counter */                               \
        /* return true if we need to switch context, otherwise false */         \
        "JSR.A  _nOS_IsrLeave               \n"                                 \
                                                                                \
        /* Do we need to switch context from ISR ? */                           \
        "CMP.B  #0, R0L                     \n"                                 \
        "JEQ    _Leave"#func"               \n"                                 \
                                                                                \
        /* YES, we need to switch context */                                    \
                                                                                \
        /* Pop all thread registers */                                          \
        /* nOS_PortContextSwitchFromIsr will push it all on USTACK */           \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
                                                                                \
        /* Switch to thread stack */                                            \
        "FSET   U                           \n"                                 \
                                                                                \
        /* Call nOS_PortContextSwitchFromIsr */                                 \
        "INT    #33                         \n"                                 \
                                                                                \
        /* NO, We DON'T need to switch context */                               \
                                                                                \
        "_Leave"#func":                     \n"                                 \
        /* Pop all registers from ISTACK */                                     \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
    );                                                                          \
}                                                                               \
void func##_L2(void)

/* Unused function */
#define nOS_PortInit()

void    nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* NOSPORT_H */
