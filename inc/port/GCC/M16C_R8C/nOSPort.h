/*
 * Copyright (c) 2014-2016 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOSPORT_H
#define NOSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t                            nOS_Stack;
typedef uint16_t                            nOS_StatusReg;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   2
#define NOS_MEM_POINTER_WIDTH               2

#define NOS_16_BITS_SCHEDULER
#define NOS_DONT_USE_CONST

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined: must be set between 1 and 7 inclusively."
#elif (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO < 1) || (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO > 7)
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is set to invalid value: must be set between 1 and 7 inclusively."
#else
 #define NOS_MAX_UNSAFE_IPL                 NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
#endif

__attribute__( ( always_inline ) ) static inline uint16_t _GetIPL(void)
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

__attribute__( ( always_inline ) ) static inline void _SetIPL(uint16_t ipl)
{
    uint16_t flg;
    __asm volatile (
        "STC    FLG,        %0      \n"
        "AND.W  #0x8FFF,    %0      \n"
        "SHL.W  #8,         %1      \n"
        "SHL.W  #4,         %1      \n"
        "OR.W   %1,         %0      \n"
        "LDC    %0,         FLG     \n"
        "NOP                        \n"
    :: "r" (flg), "r" (ipl));
}

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = _GetIPL();                                                         \
        if (sr < NOS_MAX_UNSAFE_IPL) {                                          \
            __asm volatile (                                                    \
                "LDIPL  %0              \n"                                     \
                "NOP                    \n"                                     \
            :: "i" (NOS_MAX_UNSAFE_IPL));                                       \
        }                                                                       \
    } while (0)


#define nOS_LeaveCritical(sr)                                                   \
    _SetIPL(sr)

#define nOS_SwitchContext()                             __asm volatile("INT #32")

void    nOS_EnterIsr    (void);
bool    nOS_LeaveIsr    (void);

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
        "JSR.A  _nOS_EnterIsr               \n"                                 \
                                                                                \
        /* Call user ISR function */                                            \
        "JSR.A  _"#func"_L2                 \n"                                 \
                                                                                \
        /* Ensure interrupts are disabled */                                    \
        "FCLR   I                           \n"                                 \
        "NOP                                \n"                                 \
                                                                                \
        /* Decrement interrupts nested counter */                               \
        /* return true if we need to switch context, otherwise false */         \
        "JSR.A  _nOS_LeaveIsr               \n"                                 \
                                                                                \
        /* Do we need to switch context from ISR ? */                           \
        "CMP.B  #0, R0L                     \n"                                 \
        "JEQ    _Leave"#func"               \n"                                 \
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
        "_Leave"#func":                     \n"                                 \
        /* Pop all registers from ISTACK */                                     \
        "POPM   R0,R1,R2,R3,A0,A1,SB,FB     \n"                                 \
    );                                                                          \
}                                                                               \
void func##_L2(void)

/* Unused function */
#define nOS_InitSpecific()

#ifdef NOS_PRIVATE
 void   nOS_InitContext                 (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

void    nOS_SwitchContextHandler        (void) __attribute__ ((interrupt));
void    nOS_SwitchContextFromIsrHandler (void) __attribute__ ((interrupt));

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
