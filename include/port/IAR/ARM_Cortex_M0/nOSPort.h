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

typedef uint32_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

#define NOS_PORT_MEM_ALIGNMENT              4

#define NOS_PORT_SCHED_USE_32_BITS

#ifndef NOS_CONFIG_ISR_STACK_SIZE
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined: must be higher than 0."
#elif (NOS_CONFIG_ISR_STACK_SIZE == 0)
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
#endif

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    uint32_t _primask = __get_PRIMASK();                                        \
    __disable_interrupt();                                                      \
    __DSB();                                                                    \
    __ISB()

#define nOS_CriticalLeave()                                                     \
    __set_PRIMASK(_primask);                                                    \
    __DSB();                                                                    \
    __ISB();                                                                    \
}

/*
 * Request a context switch and enable interrupts to allow PendSV interrupt.
 */
#define nOS_ContextSwitch()                                                     \
    *(volatile uint32_t *)0xe000ed04UL = 0x10000000UL;                          \
    __enable_interrupt();                                                       \
    __DSB();                                                                    \
    __ISB();                                                                    \
    __no_operation();                                                           \
    __disable_interrupt();                                                      \
    __DSB();                                                                    \
    __ISB()

void    nOS_IsrEnter    (void);
void    nOS_IsrLeave    (void);

#define NOS_ISR(func)                                                           \
void func##_ISR(void);                                                          \
void func(void)                                                                 \
{                                                                               \
    nOS_IsrEnter();                                                             \
    func##_ISR();                                                               \
    nOS_IsrLeave();                                                             \
}                                                                               \
void func##_ISR(void)

#ifdef NOS_PRIVATE
void        nOS_PortInit        (void);
#endif

void        nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
