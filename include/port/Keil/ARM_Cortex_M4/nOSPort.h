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

#define NOS_MEM_ALIGNMENT                   4
#define NOS_MEM_POINTER_WIDTH               4

#define NOS_32_BITS_SCHEDULER
#define NOS_USE_CLZ

#ifdef NOS_CONFIG_ISR_STACK_SIZE
 #if (NOS_CONFIG_ISR_STACK_SIZE == 0)
  #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
 #endif
#endif

/* __NVIC_PRIO_BITS defined from CMSIS if used */
#ifdef NOS_CONFIG_NVIC_PRIO_BITS
 #define NOS_NVIC_PRIO_BITS                 NOS_CONFIG_NVIC_PRIO_BITS
#elif defined(__NVIC_PRIO_BITS)
 #define NOS_NVIC_PRIO_BITS                 __NVIC_PRIO_BITS
#else
 #define NOS_NVIC_PRIO_BITS                 4
#endif

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined."
#endif

#define NOS_MAX_UNSAFE_BASEPRI              (NOS_CONFIG_MAX_UNSAFE_ISR_PRIO << (8 - NOS_NVIC_PRIO_BITS))

#define _CLZ(n)                             __clz(n)

#define nOS_EnterCritical()                                                     \
{                                                                               \
    uint32_t    _backup;                                                        \
    register uint32_t volatile _basepri __asm("basepri");                       \
    _backup = _basepri;                                                         \
    _basepri = NOS_MAX_UNSAFE_BASEPRI;                                          \
    __dsb(0xF);                                                                 \
    __isb(0xF)


#define nOS_LeaveCritical()                                                     \
    _basepri = _backup;                                                         \
    __dsb(0xF);                                                                 \
    __isb(0xF);                                                                 \
}

/*
 * Request a context switch and enable interrupts to allow PendSV interrupt.
 */
#define nOS_SwitchContext()                                                     \
{                                                                               \
    register uint32_t volatile _basepri __asm("basepri");                       \
    *(volatile uint32_t *)0xE000ED04UL = 0x10000000;                            \
    _basepri = 0;                                                               \
    __dsb(0xF);                                                                 \
    __isb(0xF);                                                                 \
    __nop();                                                                    \
    _basepri = NOS_MAX_UNSAFE_BASEPRI;                                          \
    __dsb(0xF);                                                                 \
    __isb(0xF);                                                                 \
}


void    nOS_EnterIsr    (void);
void    nOS_LeaveIsr    (void);

#define NOS_ISR(func)                                                           \
void func##_ISR(void);                                                          \
void func(void)                                                                 \
{                                                                               \
    nOS_EnterIsr();                                                             \
    func##_ISR();                                                               \
    nOS_LeaveIsr();                                                             \
}                                                                               \
void func##_ISR(void)

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific    (void);
 void   nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
