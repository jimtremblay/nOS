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
#include <msp430.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint16_t                nOS_Stack;

#define NOS_UNUSED(v)           (void)v

#define NOS_MEM_ALIGNMENT       2

#if defined(__MSP430X__)
#define nOS_ContextSave()                                                       \
    asm volatile (                                                              \
        "push.w     sr                          \n"                             \
        "                                       \n"                             \
        /* Push all registers to running thread stack */                        \
        "pushm.w    #12,                r15     \n"                             \
        "                                       \n"                             \
        /* Save stack pointer to running thread structure */                    \
        "mov.w      &nOS_runningThread, r12     \n"                             \
        "mov.w      sp,                 0(r12)  \n"                             \
    )

#define nOS_ContextRestore()                                                    \
    asm volatile (                                                              \
        /* Restore stack pointer from high prio thread structure */             \
        "mov.w      &nOS_runningThread, r12     \n"                             \
        "mov.w      @r12,               sp      \n"                             \
        "                                       \n"                             \
        /* Pop all registers from high prio thread stack */                     \
        "popm.w     #12,                r15     \n"                             \
        "                                       \n"                             \
        "pop.w      sr                          \n"                             \
    )
#else
#define nOS_ContextSave()                                                       \
    asm volatile (                                                              \
        "push.w     sr                          \n"                             \
        "                                       \n"                             \
        /* Push all registers to running thread stack */                        \
        "push.w     r4                          \n"                             \
        "push.w     r5                          \n"                             \
        "push.w     r6                          \n"                             \
        "push.w     r7                          \n"                             \
        "push.w     r8                          \n"                             \
        "push.w     r9                          \n"                             \
        "push.w     r10                         \n"                             \
        "push.w     r11                         \n"                             \
        "push.w     r12                         \n"                             \
        "push.w     r13                         \n"                             \
        "push.w     r14                         \n"                             \
        "push.w     r15                         \n"                             \
        "                                       \n"                             \
        /* Save stack pointer to running thread structure */                    \
        "mov.w      &nOS_runningThread, r12     \n"                             \
        "mov.w      sp,                 0(r12)  \n"                             \
    )

#define nOS_ContextRestore()                                                    \
    asm volatile (                                                              \
        /* Restore stack pointer from high prio thread structure */             \
        "mov.w      &nOS_runningThread, r12     \n"                             \
        "mov.w      @r12,               sp      \n"                             \
        "                                       \n"                             \
        /* Pop all registers from high prio thread stack */                     \
        "pop.w      r15                         \n"                             \
        "pop.w      r14                         \n"                             \
        "pop.w      r13                         \n"                             \
        "pop.w      r12                         \n"                             \
        "pop.w      r11                         \n"                             \
        "pop.w      r10                         \n"                             \
        "pop.w      r9                          \n"                             \
        "pop.w      r8                          \n"                             \
        "pop.w      r7                          \n"                             \
        "pop.w      r6                          \n"                             \
        "pop.w      r5                          \n"                             \
        "pop.w      r4                          \n"                             \
        "                                       \n"                             \
        "pop.w      sr                          \n"                             \
    )
#endif


#define nOS_CriticalEnter()                                                     \
{                                                                               \
    volatile uint16_t _sr;                                                      \
    _sr = __get_interrupt_state();                                              \
    __disable_interrupt();                                                      \
    __no_operation()


#define nOS_CriticalLeave()                                                     \
    __set_interrupt_state(_sr);                                                 \
    __no_operation();                                                           \
}

#define NOS_ISR(vect)                                                           \
void vect##_ISR_L2(void) __attribute__ ((naked));                               \
inline void vect##_ISR_L3(void) __attribute__ ((always_inline));                \
void __attribute__ ((__interrupt__(vect), naked)) vect##_ISR(void)              \
{                                                                               \
    vect##_ISR_L2();                                                            \
    asm volatile ("reti");                                                      \
}                                                                               \
void vect##_ISR_L2(void)                                                        \
{                                                                               \
    nOS_ContextSave();                                                          \
    asm volatile (                                                              \
        "mov.w      sp,                 r12     \n"                             \
        "call       #nOS_IsrEnter               \n"                             \
        "mov.w      r12,                sp      \n"                             \
    );                                                                          \
    vect##_ISR_L3();                                                            \
    __disable_interrupt();                                                      \
    __no_operation();                                                           \
    asm volatile (                                                              \
        "mov.w      sp,                 r12     \n"                             \
        "call       #nOS_IsrLeave               \n"                             \
        "mov.w      r12,                sp      \n"                             \
    );                                                                          \
    nOS_ContextRestore();                                                       \
    asm volatile ("ret");                                                       \
}                                                                               \
inline void vect##_ISR_L3(void)

/* Unused function for this port */
#define nOS_PortInit()

nOS_Stack*  nOS_IsrEnter        (nOS_Stack *sp);
nOS_Stack*  nOS_IsrLeave        (nOS_Stack *sp);

void        nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
void        nOS_ContextSwitch   (void) __attribute__ ((naked));

#if defined(__cplusplus)
}
#endif

#endif /* NOSPORT_H */
