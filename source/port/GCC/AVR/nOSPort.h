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
#include <avr/interrupt.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint8_t                 nOS_Stack;

#define NOS_UNUSED(v)           (void)v

#define NOS_PORT_MEM_ALIGNMENT  1

#if defined(__AVR_HAVE_RAMPZ__)
#define PUSH_RAMPZ()                                                \
    asm volatile (                                                  \
        "in   r0, %[RAMPZ_ADDR]             \n"                     \
        "push r0                            \n"                     \
        :: [RAMPZ_ADDR] "I" (_SFR_IO_ADDR(RAMPZ))                   \
    )
#else
#define PUSH_RAMPZ()
#endif

#if defined(__AVR_3_BYTE_PC__)
#define PUSH_EIND()                                                 \
    asm volatile (                                                  \
        "in   r0, %[EIND_ADDR]              \n"                     \
        "push r0                            \n"                     \
        :: [EIND_ADDR] "I" (_SFR_IO_ADDR(EIND))                     \
    )
#else
#define PUSH_EIND()
#endif

#if defined(__AVR_HAVE_RAMPZ__)
#define POP_RAMPZ()                                                 \
    asm volatile (                                                  \
        "pop  r0                            \n"                     \
        "out  %[RAMPZ_ADDR], r0             \n"                     \
        :: [RAMPZ_ADDR] "I" (_SFR_IO_ADDR(RAMPZ))                   \
    )
#else
#define POP_RAMPZ()
#endif

#if defined(__AVR_3_BYTE_PC__)
#define POP_EIND()                                                  \
    asm volatile (                                                  \
        "pop  r0                            \n"                     \
        "out  %[EIND_ADDR], r0              \n"                     \
        :: [EIND_ADDR] "I" (_SFR_IO_ADDR(EIND))                     \
    )
#else
#define POP_EIND()
#endif

#define nOS_CriticalEnter()                                         \
    asm volatile (                                                  \
        "in   __tmp_reg__, %[SREG_ADDR]     \n"                     \
        "cli                                \n"                     \
        "push __tmp_reg__                   \n"                     \
        :: [SREG_ADDR] "I" (_SFR_IO_ADDR(SREG))                     \
    )

#define nOS_CriticalLeave()                                         \
    asm volatile (                                                  \
        "pop  __tmp_reg__                   \n"                     \
        "out  %[SREG_ADDR], __tmp_reg__     \n"                     \
        :: [SREG_ADDR] "I" (_SFR_IO_ADDR(SREG))                     \
    )

#define nOS_ContextPush()                                           \
    asm volatile (                                                  \
        "push r0                            \n"                     \
        "in   r0, %[SREG_ADDR]              \n"                     \
        "cli                                \n"                     \
        "push r0                            \n"                     \
        :: [SREG_ADDR] "I" (_SFR_IO_ADDR(SREG))                     \
    );                                                              \
    PUSH_RAMPZ();                                                   \
    PUSH_EIND();                                                    \
    asm volatile (                                                  \
        "push r1                            \n"                     \
        "clr  r1                            \n"                     \
        "push r2                            \n"                     \
        "push r3                            \n"                     \
        "push r4                            \n"                     \
        "push r5                            \n"                     \
        "push r6                            \n"                     \
        "push r7                            \n"                     \
        "push r8                            \n"                     \
        "push r9                            \n"                     \
        "push r10                           \n"                     \
        "push r11                           \n"                     \
        "push r12                           \n"                     \
        "push r13                           \n"                     \
        "push r14                           \n"                     \
        "push r15                           \n"                     \
        "push r16                           \n"                     \
        "push r17                           \n"                     \
        "push r18                           \n"                     \
        "push r19                           \n"                     \
        "push r20                           \n"                     \
        "push r21                           \n"                     \
        "push r22                           \n"                     \
        "push r23                           \n"                     \
        "push r24                           \n"                     \
        "push r25                           \n"                     \
        "push r26                           \n"                     \
        "push r27                           \n"                     \
        "push r28                           \n"                     \
        "push r29                           \n"                     \
        "push r30                           \n"                     \
        "push r31                           \n"                     \
    )

#define nOS_ContextPop()                                            \
    asm volatile (                                                  \
        "pop  r31                           \n"                     \
        "pop  r30                           \n"                     \
        "pop  r29                           \n"                     \
        "pop  r28                           \n"                     \
        "pop  r27                           \n"                     \
        "pop  r26                           \n"                     \
        "pop  r25                           \n"                     \
        "pop  r24                           \n"                     \
        "pop  r23                           \n"                     \
        "pop  r22                           \n"                     \
        "pop  r21                           \n"                     \
        "pop  r20                           \n"                     \
        "pop  r19                           \n"                     \
        "pop  r18                           \n"                     \
        "pop  r17                           \n"                     \
        "pop  r16                           \n"                     \
        "pop  r15                           \n"                     \
        "pop  r14                           \n"                     \
        "pop  r13                           \n"                     \
        "pop  r12                           \n"                     \
        "pop  r11                           \n"                     \
        "pop  r10                           \n"                     \
        "pop  r9                            \n"                     \
        "pop  r8                            \n"                     \
        "pop  r7                            \n"                     \
        "pop  r6                            \n"                     \
        "pop  r5                            \n"                     \
        "pop  r4                            \n"                     \
        "pop  r3                            \n"                     \
        "pop  r2                            \n"                     \
        "pop  r1                            \n"                     \
    );                                                              \
    POP_EIND();                                                     \
    POP_RAMPZ();                                                    \
    asm volatile (                                                  \
        "pop  r0                            \n"                     \
        "out  %[SREG_ADDR], r0              \n"                     \
        "pop  r0                            \n"                     \
        :: [SREG_ADDR] "I" (_SFR_IO_ADDR(SREG))                     \
    )

nOS_Stack*      nOS_IsrEnter        (nOS_Stack *sp);
nOS_Stack*      nOS_IsrLeave        (nOS_Stack *sp);

#define NOS_ISR(vect)                                               \
void vect##_ISR(void) __attribute__ ( ( naked ) );                  \
inline void vect##_ISR_L2(void) __attribute__( ( always_inline ) ); \
ISR(vect, ISR_NAKED)                                                \
{                                                                   \
    vect##_ISR();                                                   \
    asm volatile ("reti");                                          \
}                                                                   \
void vect##_ISR(void)                                               \
{                                                                   \
    nOS_ContextPush();                                              \
    SP = (int)nOS_IsrEnter((nOS_Stack*)SP);                         \
    vect##_ISR_L2();                                                \
    asm volatile ("cli");                                           \
    SP = (int)nOS_IsrLeave((nOS_Stack*)SP);                         \
    nOS_ContextPop();                                               \
    asm volatile ("ret");                                           \
}                                                                   \
inline void vect##_ISR_L2(void)

void    nOS_PortInit        (void);

void    nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
/* Absolutely need a naked function because function call push the return address on the stack */
void    nOS_ContextSwitch   (void) __attribute__ ( ( naked ) );

#if defined(__cplusplus)
}
#endif

#endif /* NOSPORT_H */
