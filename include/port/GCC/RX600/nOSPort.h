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

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined."
#else
 #define NOS_PORT_MAX_UNSAFE_IPL            NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
#endif

__attribute__( ( always_inline ) ) static inline uint32_t GetIPL(void)
{
    uint32_t ipl;
    __asm volatile(
        "MVFC   PSW,            %0      \n"
        "SHLR   #24,            %0      \n"
    : "=r" (ipl) );
    return ipl;
}

__attribute__( ( always_inline ) ) static inline void SetIPL(uint32_t ipl)
{
    uint32_t psw;
    __asm volatile(
        "MVFC   PSW,            %0      \n"
        "SHLL   #24,            %1      \n"
        "AND    #0xF0FFFFFF,    %0      \n"
        "OR     %1,             %0      \n"
        "MVTC   %0,             PSW     \n"
    : "=r" (psw), "=r" (ipl) );
}

__attribute( ( always_inline ) ) static inline void EnableInterrupt(void)
{
    __asm volatile("SETPSW I");
}

__attribute( ( always_inline ) ) static inline void DisableInterrupt(void)
{
    __asm volatile("CLRPSW I");
}

#define nOS_CriticalEnter()                                                     \
{                                                                               \
    uint32_t _ipl = GetIPL();                                                   \
    if (_ipl < NOS_PORT_MAX_UNSAFE_IPL) {                                       \
        __asm volatile ("MVTIPL %0" :: "i" (NOS_PORT_MAX_UNSAFE_IPL) );         \
    }


#define nOS_CriticalLeave()                                                     \
    SetIPL(_ipl);                                                               \
}

#define nOS_ContextSwitch()                                     __asm("INT  #27")

void    nOS_IsrEnter    (void);
void    nOS_IsrLeave    (void);

#define NOS_ISR(vect)                                                           \
void vect(void) __attribute__ ( ( interrupt ) );                                \
void vect##_ISR(void);                                                          \
void vect(void)                                                                 \
{                                                                               \
    nOS_IsrEnter();                                                             \
    vect##_ISR();                                                               \
    DisableInterrupt();                                                         \
    nOS_IsrLeave();                                                             \
}                                                                               \
__attribute__ ( ( always_inline ) ) inline void vect##_ISR(void)

#ifdef NOS_PRIVATE
void    nOS_PortInit        (void);
#endif

void    nOS_ContextInit     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
