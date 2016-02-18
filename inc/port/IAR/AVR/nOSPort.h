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

typedef uint8_t                         nOS_Stack;
typedef uint8_t                         nOS_StatusReg;

#define NOS_UNUSED(v)                   (void)v

#define NOS_MEM_ALIGNMENT               1

#define NOS_USE_SEPARATE_CALL_STACK

#ifdef __HAS_RAMPZ__
 #define PUSH_RAMPZ                                                             \
    "in     r0,     0x3B                \n"                                     \
    "st     -y,     r0                  \n"
#else
 #define PUSH_RAMPZ
#endif

#if defined(__ATmega2560__) || defined(__ATmega2561__)
 #define PUSH_EIND                                                              \
    "in     r0,     0x3C                \n"                                     \
    "st     -y,     r0                  \n"
#else
 #define PUSH_EIND
#endif

#ifdef __HAS_RAMPZ__
 #define POP_RAMPZ                                                              \
    "ld     r0,     y+                  \n"                                     \
    "out    0x3B,   r0                  \n"
#else
 #define POP_RAMPZ
#endif

#if defined(__ATmega2560__) || defined(__ATmega2561__)
 #define POP_EIND                                                               \
    "ld     r0,     y+                  \n"                                     \
    "out    0x3C,   r0                  \n"
#else
 #define POP_EIND
#endif

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = __save_interrupt();                                                \
        __disable_interrupt();                                                  \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    __restore_interrupt(sr)

#define PUSH_CONTEXT()                                                          \
    __asm (                                                                     \
        "st     -y,     r0                  \n" /* R0 */                        \
        "in     r0,     0x3F                \n"                                 \
        "cli                                \n"                                 \
        "st     -y,     r0                  \n" /* SREG */                      \
        "in     r0,     0x3D                \n"                                 \
        "st     -y,     r0                  \n" /* SPL */                       \
        "in     r0,     0x3E                \n"                                 \
        "st     -y,     r0                  \n" /* SPH */                       \
        PUSH_RAMPZ                              /* RAMPZ */                     \
        PUSH_EIND                               /* EIND */                      \
        "st     -y,     r1                  \n" /* R1 */                        \
        "st     -y,     r2                  \n" /* R2 */                        \
        "st     -y,     r3                  \n" /* R3 */                        \
        "st     -y,     r4                  \n" /* R4 */                        \
        "st     -y,     r5                  \n" /* R5 */                        \
        "st     -y,     r6                  \n" /* R6 */                        \
        "st     -y,     r7                  \n" /* R7 */                        \
        "st     -y,     r8                  \n" /* R8 */                        \
        "st     -y,     r9                  \n" /* R9 */                        \
        "st     -y,     r10                 \n" /* R10 */                       \
        "st     -y,     r11                 \n" /* R11 */                       \
        "st     -y,     r12                 \n" /* R12 */                       \
        "st     -y,     r13                 \n" /* R13 */                       \
        "st     -y,     r14                 \n" /* R14 */                       \
        "st     -y,     r15                 \n" /* R15 */                       \
        "st     -y,     r16                 \n" /* R16 */                       \
        "st     -y,     r17                 \n" /* R17 */                       \
        "st     -y,     r18                 \n" /* R18 */                       \
        "st     -y,     r19                 \n" /* R19 */                       \
        "st     -y,     r20                 \n" /* R20 */                       \
        "st     -y,     r21                 \n" /* R21 */                       \
        "st     -y,     r22                 \n" /* R22 */                       \
        "st     -y,     r23                 \n" /* R23 */                       \
        "st     -y,     r24                 \n" /* R24 */                       \
        "st     -y,     r25                 \n" /* R25 */                       \
        "st     -y,     r26                 \n" /* R26 */                       \
        "st     -y,     r27                 \n" /* R27 */                       \
        "st     -y,     r30                 \n" /* R30 */                       \
        "st     -y,     r31                 \n" /* R31 */                       \
    )

#define POP_CONTEXT()                                                           \
    __asm (                                                                     \
        "ld     r31,    y+                  \n" /* R31 */                       \
        "ld     r30,    y+                  \n" /* R30 */                       \
        "ld     r27,    y+                  \n" /* R27 */                       \
        "ld     r26,    y+                  \n" /* R26 */                       \
        "ld     r25,    y+                  \n" /* R25 */                       \
        "ld     r24,    y+                  \n" /* R24 */                       \
        "ld     r23,    y+                  \n" /* R23 */                       \
        "ld     r22,    y+                  \n" /* R22 */                       \
        "ld     r21,    y+                  \n" /* R21 */                       \
        "ld     r20,    y+                  \n" /* R20 */                       \
        "ld     r19,    y+                  \n" /* R19 */                       \
        "ld     r18,    y+                  \n" /* R18 */                       \
        "ld     r17,    y+                  \n" /* R17 */                       \
        "ld     r16,    y+                  \n" /* R16 */                       \
        "ld     r15,    y+                  \n" /* R15 */                       \
        "ld     r14,    y+                  \n" /* R14 */                       \
        "ld     r13,    y+                  \n" /* R13 */                       \
        "ld     r12,    y+                  \n" /* R12 */                       \
        "ld     r11,    y+                  \n" /* R11 */                       \
        "ld     r10,    y+                  \n" /* R10 */                       \
        "ld     r9,     y+                  \n" /* R9 */                        \
        "ld     r8,     y+                  \n" /* R8 */                        \
        "ld     r7,     y+                  \n" /* R7 */                        \
        "ld     r6,     y+                  \n" /* R6 */                        \
        "ld     r5,     y+                  \n" /* R5 */                        \
        "ld     r4,     y+                  \n" /* R4 */                        \
        "ld     r3,     y+                  \n" /* R3 */                        \
        "ld     r2,     y+                  \n" /* R2 */                        \
        "ld     r1,     y+                  \n" /* R1 */                        \
        POP_EIND                                /* EIND */                      \
        POP_RAMPZ                               /* RAMPZ */                     \
        "ld     r0,     y+                  \n" /* SPH */                       \
        "out    0x3E,   r0                  \n"                                 \
        "ld     r0,     y+                  \n" /* SPL */                       \
        "out    0x3D,   r0                  \n"                                 \
        "ld     r0,     y+                  \n" /* SREG */                      \
        "out    0x3F,   r0                  \n"                                 \
        "ld     r0,     y+                  \n" /* R0 */                        \
    )

void            nOS_EnterIsr        (nOS_Stack *sp);
nOS_Stack*      nOS_LeaveIsr        (nOS_Stack *sp);

__no_init volatile uint16_t RSP @ 0x1C;

#define NOS_ISR(vect)                                                           \
__task void vect##_ISR_L2(void);                                                \
__task void vect##_ISR_L3(void);                                                \
_Pragma(_STRINGIFY(vector=##vect))                                              \
__raw __interrupt void vect##_ISR(void)                                         \
{                                                                               \
    vect##_ISR_L2();                                                            \
    /* reti */                                                                  \
}                                                                               \
__task void vect##_ISR_L2(void)                                                 \
{                                                                               \
    PUSH_CONTEXT();                                                             \
    nOS_EnterIsr((nOS_Stack*)RSP);                                              \
    vect##_ISR_L3();                                                            \
    __disable_interrupt();                                                      \
    RSP = (uint16_t)nOS_LeaveIsr((nOS_Stack*)RSP);                              \
    POP_CONTEXT();                                                              \
    /* ret */                                                                   \
}                                                                               \
__task void vect##_ISR_L3(void)

/* Unused function for this port */
#define     nOS_InitSpecific()

#ifdef NOS_PRIVATE
 void           nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, size_t cssize, nOS_ThreadEntry entry, void *arg);
 /* Absolutely need a naked function because function call push the return address on the hardware stack */
 __task void    nOS_SwitchContext   (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
