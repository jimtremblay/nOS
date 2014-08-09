/*
 * Copyright (c) 2014 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IAR_CORTEX_M0_H
#define IAR_CORTEX_M0_H

#include <stdint.h>
#include <intrinsics.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint32_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

#define NOS_MEM_ALIGNMENT                   4

#define NOS_PORT_SCHED_USE_32_BITS

#if !defined(NOS_CONFIG_ISR_STACK_SIZE)
 #define NOS_CONFIG_ISR_STACK_SIZE          128
 #if defined(NOS_USE_CONFIG_FILE)
  #warning "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined (default to 128)."
 #endif
#else
 #if NOS_CONFIG_ISR_STACK_SIZE == 0
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #endif
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

#if defined(NOS_PRIVATE)
void        nOS_PortInit        (void);
#endif  /* NOS_PRIVATE */

void        nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, void(*func)(void*), void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* IAR_CORTEX_M3_H */
