/*
 * Copyright (c) 2014-2016 Jim Tremblay
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

typedef uint32_t                        nOS_Stack;
typedef uint32_t                        nOS_StatusReg;

#define NOS_UNUSED(v)                   (void)v

#define NOS_MEM_ALIGNMENT               4
#define NOS_MEM_POINTER_WIDTH           4

#define NOS_32_BITS_SCHEDULER

#ifndef NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
 #error "nOSConfig.h: NOS_CONFIG_MAX_UNSAFE_ISR_PRIO is not defined."
#else
 #define NOS_MAX_UNSAFE_IPL             NOS_CONFIG_MAX_UNSAFE_ISR_PRIO
#endif

__attribute__( ( always_inline ) ) static inline uint32_t _GetIPL(void)
{
    uint32_t ipl;
    __asm volatile(
        "MVFC   PSW,            %0      \n"
        "SHLR   #24,            %0      \n"
    : "=r" (ipl) );
    return ipl;
}

__attribute__( ( always_inline ) ) static inline void _SetIPL(uint32_t ipl)
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

__attribute( ( always_inline ) ) static inline void _EI(void)
{
    __asm volatile("SETPSW I");
}

__attribute( ( always_inline ) ) static inline void _DI(void)
{
    __asm volatile("CLRPSW I");
}

#define nOS_EnterCritical(sr)                                                   \
    do {                                                                        \
        sr = _GetIPL();                                                         \
        if (sr < NOS_MAX_UNSAFE_IPL) {                                          \
            __asm volatile ("MVTIPL %0" :: "i" (NOS_MAX_UNSAFE_IPL) );          \
        }                                                                       \
    } while (0)

#define nOS_LeaveCritical(sr)                                                   \
    _SetIPL(sr)

#define nOS_SwitchContext()                                     __asm("INT  #27")

void    nOS_EnterIsr    (void);
void    nOS_LeaveIsr    (void);

#define NOS_ISR(vect)                                                           \
void vect(void) __attribute__ ( ( interrupt ) );                                \
void vect##_ISR(void);                                                          \
void vect(void)                                                                 \
{                                                                               \
    nOS_EnterIsr();                                                             \
    vect##_ISR();                                                               \
    _DI();                                                                      \
    nOS_LeaveIsr();                                                             \
}                                                                               \
__attribute__ ( ( always_inline ) ) inline void vect##_ISR(void)

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific    (void);
 void   nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
