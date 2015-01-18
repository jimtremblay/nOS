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
 #warning "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined (default to 128)."
#else
 #if NOS_CONFIG_ISR_STACK_SIZE == 0
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value."
 #endif
#endif

#if !defined(NOS_CONFIG_MAX_UNSAFE_ISR_PRIO)
 #define NOS_CONFIG_MAX_UNSAFE_ISR_PRIO     4
 #warning "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined (default to 4)."
#endif

#define NOS_PORT_MAX_UNSAFE_IPL             NOS_CONFIG_MAX_UNSAFE_ISR_PRIO

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    __ilevel_t _ipl = __get_interrupt_level();                                  \
    if (_ipl < NOS_PORT_MAX_UNSAFE_IPL) {                                       \
        __set_interrupt_level(NOS_PORT_MAX_UNSAFE_IPL);                         \
    }

#define nOS_CriticalLeave()                                                     \
    __set_interrupt_level(_ipl);                                                \
}

#define nOS_ContextSwitch()                                     __asm("INT  #27")

void    nOS_IsrEnter    (void);
void    nOS_IsrLeave    (void);

#define NOS_ISR(vect)                                                           \
void vect##_ISR_L2(void);                                                       \
_Pragma(_STRINGIFY(vector=vect))                                                \
__interrupt void vect##_ISR(void)                                               \
{                                                                               \
    nOS_IsrEnter();                                                             \
    vect##_ISR_L2();                                                            \
    __disable_interrupt();                                                      \
    nOS_IsrLeave();                                                             \
}                                                                               \
inline void vect##_ISR_L2(void)

#if defined(NOS_PRIVATE)
void    nOS_PortInit        (void);
#endif

void    nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* NOSPORT_H */
