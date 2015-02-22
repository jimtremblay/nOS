/*
 * Copyright (c) 2014-2015 Jim Tremblay
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

typedef uint32_t                            nOS_Stack;

#define NOS_UNUSED(v)                       (void)v

#define NOS_PORT_MEM_ALIGNMENT              4

#define NOS_PORT_SCHED_USE_32_BITS
#define NOS_PORT_HAVE_CLZ

/* __NVIC_PRIO_BITS defined from CMSIS if used */
#ifdef NOS_CONFIG_NVIC_PRIO_BITS
 #define NOS_NVIC_PRIO_BITS                 NOS_CONFIG_NVIC_PRIO_BITS
#elif defined(__NVIC_PRIO_BITS)
 #define NOS_NVIC_PRIO_BITS                 __NVIC_PRIO_BITS
#else
 #define NOS_NVIC_PRIO_BITS                 4
#endif

#ifndef NOS_CONFIG_ISR_STACK_SIZE
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined: must be higher than 0."
#elif (NOS_CONFIG_ISR_STACK_SIZE == 0)
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
#endif

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined."
#endif

#define NOS_PORT_MAX_UNSAFE_BASEPRI         (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << (8 - NOS_NVIC_PRIO_BITS))

#define nOS_PortCLZ(n)                      __clz(n)

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    uint32_t    _backup;                                                        \
    register uint32_t volatile _basepri __asm("basepri");                       \
    _backup = _basepri;                                                         \
    _basepri = NOS_PORT_MAX_UNSAFE_BASEPRI;                                     \
    __dsb(0xf);                                                                 \
    __isb(0xf);                                                                 \


#define nOS_CriticalLeave()                                                     \
    _basepri = _backup;                                                         \
    __dsb(0xf);                                                                 \
    __isb(0xf);                                                                 \
}

/*
 * Request a context switch and enable interrupts to allow PendSV interrupt.
 */
#define nOS_ContextSwitch()                                                     \
{                                                                               \
    register uint32_t volatile _basepri __asm("basepri");                       \
    *(volatile uint32_t *)0xe000ed04UL = 0x10000000UL;                          \
    _basepri = 0;                                                               \
    __dsb(0xf);                                                                 \
    __isb(0xf);                                                                 \
    __nop();                                                                    \
    _basepri = NOS_PORT_MAX_UNSAFE_BASEPRI;                                     \
    __dsb(0xf);                                                                 \
    __isb(0xf);                                                                 \
}


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
