/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GCC_AVR_H
#define GCC_AVR_H

#include <stdint.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t                 stack_t;

#define NOS_STACK(s,l)          stack_t s[l]

#define NOS_UNUSED(v)           (void)v

#define NOS_MEM_ALIGNMENT       1

#ifdef __AVR_HAVE_RAMPZ__
#define PUSH_RAMPZ                                                  \
    asm volatile ("in   r0, %0                      \n\t"           \
                  "push r0                          \n\t"           \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(RAMPZ)));
#else
#define PUSH_RAMPZ
#endif

#ifdef __AVR_3_BYTE_PC__
#define PUSH_EIND                                                   \
    asm volatile ("in   r0, %0                      \n\t"           \
                  "push r0                          \n\t"           \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(EIND)));
#else
#define PUSH_EIND
#endif

#ifdef __AVR_HAVE_RAMPZ__
#define POP_RAMPZ                                                   \
    asm volatile ("pop  r0                          \n\t"           \
                  "out  %0, r0                      \n\t"           \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(RAMPZ)));
#else
#define POP_RAMPZ
#endif

#ifdef __AVR_3_BYTE_PC__
#define POP_EIND                                                    \
    asm volatile ("pop  r0                          \n\t"           \
                  "out  %0, r0                      \n\t"           \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(EIND)));
#else
#define POP_EIND
#endif

#define nOS_CriticalEnter()                                         \
    asm volatile ("in   __tmp_reg__, %0         \n\t"               \
                  "cli                          \n\t"               \
                  "push __tmp_reg__             \n\t"               \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(SREG)))

#define nOS_CriticalLeave()                                         \
    asm volatile ("pop  __tmp_reg__             \n\t"               \
                  "out  %0, __tmp_reg__         \n\t"               \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(SREG)))

#define nOS_ContextPush()                                           \
    asm volatile ("push r0                          \n\t"           \
                  "in   r0, %0                      \n\t"           \
                  "cli                              \n\t"           \
                  "push r0                          \n\t"           \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(SREG)));                      \
                  PUSH_RAMPZ                                        \
                  PUSH_EIND                                         \
    asm volatile ("push r1                          \n\t"           \
                  "clr  r1                          \n\t"           \
                  "push r2                          \n\t"           \
                  "push r3                          \n\t"           \
                  "push r4                          \n\t"           \
                  "push r5                          \n\t"           \
                  "push r6                          \n\t"           \
                  "push r7                          \n\t"           \
                  "push r8                          \n\t"           \
                  "push r9                          \n\t"           \
                  "push r10                         \n\t"           \
                  "push r11                         \n\t"           \
                  "push r12                         \n\t"           \
                  "push r13                         \n\t"           \
                  "push r14                         \n\t"           \
                  "push r15                         \n\t"           \
                  "push r16                         \n\t"           \
                  "push r17                         \n\t"           \
                  "push r18                         \n\t"           \
                  "push r19                         \n\t"           \
                  "push r20                         \n\t"           \
                  "push r21                         \n\t"           \
                  "push r22                         \n\t"           \
                  "push r23                         \n\t"           \
                  "push r24                         \n\t"           \
                  "push r25                         \n\t"           \
                  "push r26                         \n\t"           \
                  "push r27                         \n\t"           \
                  "push r28                         \n\t"           \
                  "push r29                         \n\t"           \
                  "push r30                         \n\t"           \
                  "push r31                         \n\t")

#define nOS_ContextPop()                                            \
    asm volatile ("pop  r31                         \n\t"           \
                  "pop  r30                         \n\t"           \
                  "pop  r29                         \n\t"           \
                  "pop  r28                         \n\t"           \
                  "pop  r27                         \n\t"           \
                  "pop  r26                         \n\t"           \
                  "pop  r25                         \n\t"           \
                  "pop  r24                         \n\t"           \
                  "pop  r23                         \n\t"           \
                  "pop  r22                         \n\t"           \
                  "pop  r21                         \n\t"           \
                  "pop  r20                         \n\t"           \
                  "pop  r19                         \n\t"           \
                  "pop  r18                         \n\t"           \
                  "pop  r17                         \n\t"           \
                  "pop  r16                         \n\t"           \
                  "pop  r15                         \n\t"           \
                  "pop  r14                         \n\t"           \
                  "pop  r13                         \n\t"           \
                  "pop  r12                         \n\t"           \
                  "pop  r11                         \n\t"           \
                  "pop  r10                         \n\t"           \
                  "pop  r9                          \n\t"           \
                  "pop  r8                          \n\t"           \
                  "pop  r7                          \n\t"           \
                  "pop  r6                          \n\t"           \
                  "pop  r5                          \n\t"           \
                  "pop  r4                          \n\t"           \
                  "pop  r3                          \n\t"           \
                  "pop  r2                          \n\t"           \
                  "pop  r1                          \n\t");         \
                  POP_EIND                                          \
                  POP_RAMPZ                                         \
    asm volatile ("pop  r0                          \n\t"           \
                  "out  %0, r0                      \n\t"           \
                  "pop  r0                          \n\t"           \
                  :                                                 \
                  : "I" (_SFR_IO_ADDR(SREG)))

stack_t*    nOS_IsrEnter        (stack_t *sp);
stack_t*    nOS_IsrLeave        (stack_t *sp);

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
    SP = (int)nOS_IsrEnter((stack_t*)SP);                           \
    vect##_ISR_L2();                                                \
    asm volatile ("cli");                                           \
    SP = (int)nOS_IsrLeave((stack_t*)SP);                           \
    nOS_ContextPop();                                               \
    asm volatile ("ret");                                           \
}                                                                   \
inline void vect##_ISR_L2(void)

/* Unused function for this port */
#define nOS_PortInit()

void        nOS_ContextInit     (nOS_Thread *thread, uint8_t *stack, size_t ssize, void(*func)(void*), void *arg);
/* Absolutely need a naked function because function call push the return address on the stack */
void        nOS_ContextSwitch   (void) __attribute__ ( ( naked ) );

#ifdef __cplusplus
}
#endif

#endif /* GCC_AVR_H */
