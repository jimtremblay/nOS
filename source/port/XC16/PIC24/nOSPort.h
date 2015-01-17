/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include <xc.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint16_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   2

#define NOS_PORT_SCHED_USE_16_BITS

#if !defined(NOS_CONFIG_ISR_STACK_SIZE)
 #define NOS_CONFIG_ISR_STACK_SIZE          0
 #warning "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined (disabled by default)."
#endif

#if !defined(NOS_CONFIG_MAX_UNSAFE_ISR_PRIO)
 #define NOS_CONFIG_MAX_UNSAFE_ISR_PRIO     4
 #warning "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined (default to 4)."
#elif (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO == 0) || (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO >= 7)
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is set to invalid value."
#endif

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    uint16_t _sr;                                                               \
    _sr = SRbits.IPL;                                                           \
    if (SRbits.IPL < NOS_CONFIG_MAX_UNSAFE_ISR_PRIO) {                          \
        SRbits.IPL = NOS_CONFIG_MAX_UNSAFE_ISR_PRIO;                            \
    }

#define nOS_CriticalLeave()                                                     \
    SRbits.IPL = _sr;                                                           \
}

#if defined(__HAS_EDS__)
#define PUSH_PAGE_ADDRESS                                                       \
    "PUSH   DSRPAG                      \n"                                     \
    "PUSH   DSWPAG                      \n"
#define POP_PAGE_ADDRESS                                                        \
    "POP    DSWPAG                      \n"                                     \
    "POP    DSRPAG                      \n"
#else
#define PUSH_PAGE_ADDRESS                                                       \
    "PUSH   PSVPAG                      \n"
#define POP_PAGE_ADDRESS                                                        \
    "POP    PSVPAG                      \n"
#endif

#define NOS_ISR(vect)                                                           \
void __attribute__((naked)) vect##_ISR(void);                                   \
void __attribute__((naked)) vect##_ISR_L2(void);                                \
nOS_Stack* __attribute__((keep)) nOS_IsrEnter(nOS_Stack*);                      \
nOS_Stack* __attribute__((keep)) nOS_IsrLeave(nOS_Stack*);                      \
void __attribute__((__interrupt__, auto_psv, naked)) vect(void)                 \
{                                                                               \
    vect##_ISR();                                                               \
}                                                                               \
void __attribute__((naked)) vect##_ISR(void)                                    \
{                                                                               \
    __asm volatile (                                                            \
        /* Push status register */                                              \
        "PUSH   SR                          \n"                                 \
        /* Push registers W0-W1 */                                              \
        "PUSH.D W0                          \n"                                 \
        /* Disable interrupts */                                                \
        "MOV    #%0,    W0                  \n"                                 \
        "MOV    W0,     SR                  \n"                                 \
        /* Push registers W2-W13 */                                             \
        "PUSH.D W2                          \n"                                 \
        "PUSH.D W4                          \n"                                 \
        "PUSH.D W6                          \n"                                 \
        "PUSH.D W8                          \n"                                 \
        "PUSH.D W10                         \n"                                 \
        "PUSH.D W12                         \n"                                 \
        /* Push frame pointer register */                                       \
        "PUSH   W14                         \n"                                 \
        /* Push REPEAT loop counter register */                                 \
        "PUSH   RCOUNT                      \n"                                 \
        /* Push data table page address register */                             \
        "PUSH   TBLPAG                      \n"                                 \
        /* Push CPU core control register */                                    \
        "PUSH   CORCON                      \n"                                 \
        /* Push page address register */                                        \
        PUSH_PAGE_ADDRESS                                                       \
        /* Save stack pointer if first nested interrupt */                      \
        "MOV    W15,                    W0  \n"                                 \
        "CALL   _nOS_IsrEnter               \n"                                 \
        "MOV    W0,                     W15 \n"                                 \
        :: "i" (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << 5));                          \
    /* Call user ISR */                                                         \
    vect##_ISR_L2();                                                            \
    __asm volatile (                                                            \
        /* Ensure interrupts are disabled */                                    \
        "MOV    #%0,    W0                  \n"                                 \
        "MOV    W0,     SR                  \n"                                 \
        /* Restore stack pointer if last nested interrupt */                    \
        "MOV    W15,                    W0  \n"                                 \
        "CALL   _nOS_IsrLeave               \n"                                 \
        "MOV    W0,                     W15 \n"                                 \
        /* Pop page address register */                                         \
        POP_PAGE_ADDRESS                                                        \
        /* Pop CPU core control register */                                     \
        "POP    CORCON                      \n"                                 \
        /* Pop data table page address register */                              \
        "POP    TBLPAG                      \n"                                 \
        /* Pop REPEAT loop counter register */                                  \
        "POP    RCOUNT                      \n"                                 \
        /* Pop frame pointer register */                                        \
        "POP    W14                         \n"                                 \
        /* Pop registers W0-W13 */                                              \
        "POP.D  W12                         \n"                                 \
        "POP.D  W10                         \n"                                 \
        "POP.D  W8                          \n"                                 \
        "POP.D  W6                          \n"                                 \
        "POP.D  W4                          \n"                                 \
        "POP.D  W2                          \n"                                 \
        "POP.D  W0                          \n"                                 \
        /* Pop status register */                                               \
        "POP    SR                          \n"                                 \
        :: "i" (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << 5));                          \
}                                                                               \
void __attribute__((naked)) vect##_ISR_L2(void)


nOS_Stack *nOS_IsrEnter (nOS_Stack *sp);
nOS_Stack *nOS_IsrLeave (nOS_Stack *sp);

#define nOS_PortInit()

void        nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
void        nOS_ContextSwitch   (void);

#if defined(__cplusplus)
}
#endif

#endif /* PORT_H */
