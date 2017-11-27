/*
 * Copyright (c) 2018 Alain Royer, Jim Tremblay
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
typedef uint32_t                            nOS_StatusReg;

#define NOS_UNUSED(v)                       ((void)v)

#define NOS_MEM_ALIGNMENT                   4
#define NOS_MEM_POINTER_WIDTH               4

#define NOS_32_BITS_SCHEDULER

#ifndef NOS_CONFIG_ISR_STACK_SIZE
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is not defined: must be higher than 0."
#elif (NOS_CONFIG_ISR_STACK_SIZE == 0)
 #error "nOSConfig.h: NOS_CONFIG_ISR_STACK_SIZE is set to invalid value: must be higher than 0."
#endif

__attribute__( ( always_inline ) ) static inline uint32_t _GetCPSR (void)
{
    uint32_t r;
    __asm volatile ("MRS %0, CPSR" : "=r" (r));
    return r;
}

__attribute__( ( always_inline ) ) static inline void _SetCPSR (uint32_t r)
{
    __asm volatile ("MSR CPSR, %0" :: "r" (r));
}

__attribute__( ( always_inline ) ) static inline void _NOP (void)
{
    __asm volatile ("NOP");
}

__attribute__( ( always_inline ) ) static inline void nOS_SwitchContext (void)
{
    __asm volatile ("SWI 0x00");
}

__attribute__( ( always_inline ) ) static inline void _DI (void)
{
    __asm volatile (
        "STMDB  SP!, {R0}                           \n"
        "MRS    R0, CPSR                            \n"
        "ORR    R0, R0, #0xC0                       \n"
        "MSR    CPSR, R0                            \n"
        "LDMIA  SP!, {R0}                           \n"
    );
}

__attribute__( ( always_inline ) ) static inline void _EI (void)
{
    __asm volatile (
        "STMDB  SP!, {R0}                           \n"
        "MRS    R0, CPSR                            \n"
        "BIC    R0, R0, #0xC0                       \n"
        "MSR    CPSR, R0                            \n"
        "LDMIA  SP!, {R0}                           \n"
    );
}

#define PUSH_CONTEXT()                                                              \
{                                                                                   \
    extern nOS_Thread *nOS_runningThread;                                           \
    __asm volatile (                                                                \
        /* We need R0 so we save it. */                                             \
        "STMDB  SP!, {R0}                           \n"                             \
                                                                                    \
        /* R0 will point on the user stack pointer */                               \
        "STMDB  SP,{SP}^                            \n"                             \
        "NOP                                        \n"                             \
        "SUB    SP, SP, #4                          \n"                             \
        /* System stack hold the stack user pointer */                              \
        "LDMIA  SP!,{R0}                            \n"                             \
                                                                                    \
        /* Save the return address onto the thread stack. */                        \
        "STMDB  R0!, {LR}                           \n"                             \
                                                                                    \
        /* We use LR from this point.*/                                             \
        "MOV    LR, R0                              \n"                             \
                                                                                    \
        /* Get previously save R0, and save it on the thread stack. */              \
        "LDMIA  SP!, {R0}                           \n"                             \
                                                                                    \
        /* We save all the thread registers on the thread stack. */                 \
        "STMDB  LR, {R0-LR}^                        \n"                             \
        "NOP                                        \n"                             \
        "SUB    LR, LR, #60                         \n"                             \
                                                                                    \
        /* Save the SPSR on the thread stack. */                                    \
        "MRS    R0, SPSR                            \n"                             \
        "STMDB  LR!, {R0}                           \n"                             \
                                                                                    \
        /* Get the address of nOS_runningThread */                                  \
        "LDR    R0, =nOS_runningThread              \n"                             \
        "LDR    R0, [R0]                            \n"                             \
        /* Save user stack pointer to nOS_Thread object of current running thread */\
        "STR    LR, [R0]                            \n"                             \
    );                                                                              \
}

#define POP_CONTEXT()                                                               \
{                                                                                   \
    extern nOS_Thread *nOS_runningThread;                                           \
    __asm volatile (                                                                \
        /* Get the address of nOS_runningThread */                                  \
        "LDR    R0, =nOS_runningThread              \n"                             \
        "LDR    R0, [R0]                            \n"                             \
        /* Get user stack pointer from nOS_Thread object of running thread */       \
        "LDR    LR, [R0]                            \n"                             \
                                                                                    \
        /* Get the SPSR from the thread stack. */                                   \
        "LDMIA  LR!, {R0}                           \n"                             \
        "MSR    SPSR, R0                            \n"                             \
                                                                                    \
        /* Get all the thread registers from the thread stack. */                   \
        "LDMIA  LR, {R0-R14}^                       \n"                             \
        "NOP                                        \n"                             \
                                                                                    \
        /* Get the return address. */                                               \
        "LDR    LR, [LR, #60]                       \n"                             \
                                                                                    \
        /* Correcting the offset in LR and return. */                               \
        "SUBS   PC, LR, #4                          \n"                             \
    );                                                                              \
}


#define nOS_EnterCritical(sr)                                                       \
    do {                                                                            \
        sr = _GetCPSR();                                                            \
        _DI();                                                                      \
    } while (0)


#define nOS_LeaveCritical(sr)                                                       \
    do {                                                                            \
        _SetCPSR (sr);                                                              \
    } while (0)


void            nOS_EnterIsr        (void);
void            nOS_LeaveIsr        (void);
void            SWI_Handler         (void)              __attribute__( ( interrupt("SWI"), naked ) );

#if (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0) || (NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE > 0)
#define NOS_ISR(func)                                                               \
void func(void) __attribute__ ( ( naked ) );                                        \
void func##_ISR(void) __attribute__ ( ( always_inline ) );                          \
void func(void)                                                                     \
{                                                                                   \
    PUSH_CONTEXT();                                                                 \
    nOS_EnterIsr();                                                                 \
    func##_ISR();                                                                   \
    nOS_LeaveIsr();                                                                 \
    VICVectAddr = 0;                                                                \
    POP_CONTEXT();                                                                  \
}                                                                                   \
inline void func##_ISR(void)
#else
#define NOS_ISR(func)                                                               \
void func(void) __attribute__ ( ( interrupt("IRQ") ) );                             \
void func##_ISR(void) __attribute__ ( ( always_inline ) );                          \
void func(void)                                                                     \
{                                                                                   \
    nOS_EnterIsr();                                                                 \
    func##_ISR();                                                                   \
    nOS_LeaveIsr();                                                                 \
    VICVectAddr = 0;                                                                \
}                                                                                   \
inline void func##_ISR(void)
#endif

#ifdef NOS_PRIVATE
 void   nOS_InitSpecific    (void);
 void   nOS_InitContext     (nOS_Thread *thread, nOS_Stack *stack, size_t ssize, nOS_ThreadEntry entry, void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOSPORT_H */
