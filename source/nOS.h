/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef NOS_H
#define NOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#define __C99_COMPLIANT__       0
#ifdef __STDC__
 #ifdef __STDC_VERSION__
  #if (__STDC_VERSION__ >= 199901L)
   #undef  __C99_COMPLIANT__
   #define __C99_COMPLIANT__    1
  #endif
 #endif
#endif

#if (__C99_COMPLIANT__ > 0)
 #include <stdint.h>
 #include <stdbool.h>
#else
 #define bool               _Bool
 #define false              0
 #define true               1
 typedef signed char        int8_t;
 typedef unsigned char      uint8_t;
 typedef signed short       int16_t;
 typedef unsigned short     uint16_t;
 typedef signed long        int32_t;
 typedef unsigned long      uint32_t;
 typedef signed long long   int64_t;
 typedef unsigned long long uint64_t;
#endif

#ifndef UINT8_MAX
#define UINT8_MAX                   255U
#endif

#ifndef UINT16_MAX
#define UINT16_MAX                  65535U
#endif

#ifndef UINT32_MAX
#define UINT32_MAX                  4294967295UL
#endif

#ifndef UINT64_MAX
#define UINT64_MAX                  18446744073709551615ULL
#endif

#define NOS_QUOTE(s)                #s
#define NOS_STR(s)                  NOS_QUOTE(s)
#define NOS_VERSION                 NOS_STR(NOS_VERSION_MAJOR)"."NOS_STR(NOS_VERSION_MINOR)"."NOS_STR(NOS_VERSION_BUILD)

#define NOS_VERSION_MAJOR           0
#define NOS_VERSION_MINOR           1
#define NOS_VERSION_BUILD           0

#ifdef NOS_GLOBALS
 #define NOS_EXTERN
#else
 #define NOS_EXTERN                 extern
#endif

#include "nOSConfig.h"

#ifndef NOS_CONFIG_DEBUG
 #error "nOSConfig.h: NOS_CONFIG_DEBUG is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_DEBUG != 0) && (NOS_CONFIG_DEBUG != 1)
 #error "nOSConfig.h: NOS_CONFIG_DEBUG is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_SAFE
 #error "nOSConfig.h: NOS_CONFIG_SAFE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_SAFE != 0) && (NOS_CONFIG_SAFE != 1)
 #error "nOSConfig.h: NOS_CONFIG_SAFE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_HIGHEST_THREAD_PRIO
 #error "nOSConfig.h: NOS_CONFIG_HIGHEST_THREAD_PRIO is not defined: must be set between 0 and 255 inclusively."
#elif (NOS_CONFIG_HIGHEST_THREAD_PRIO < 0) || (NOS_CONFIG_HIGHEST_THREAD_PRIO > 255)
 #error "nOSConfig.h: NOS_CONFIG_HIGHEST_THREAD_PRIO is set to invalid value: must be set between 0 and 255 inclusively."
#endif

#ifndef NOS_CONFIG_TICK_COUNT_WIDTH
 #error "nOSConfig.h: NOS_CONFIG_TICK_COUNT_WIDTH is not defined: must be set to 8, 16, 32 or 64."
#elif (NOS_CONFIG_TICK_COUNT_WIDTH != 8) && (NOS_CONFIG_TICK_COUNT_WIDTH != 16) && (NOS_CONFIG_TICK_COUNT_WIDTH != 32) && (NOS_CONFIG_TICK_COUNT_WIDTH != 64)
 #error "nOSConfig.h: NOS_CONFIG_TICK_COUNT_WIDTH is set to invalid value: must be set to 8, 16, 32 or 64."
#endif

#ifndef NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
  #error "nOSConfig.h: NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE is not defined: must be set to 0 or 1."
 #endif
#elif (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE != 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0) && (NOS_CONFIG_HIGHEST_THREAD_PRIO == 0)
 #error "nOSConfig.h: NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE can't be use when NOS_CONFIG_HIGHEST_THREAD_PRIO == 0 (cooperative scheduling)."
#endif

#ifndef NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE != 0) && (NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_SCHED_ROUND_ROBIN_ENABLE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_SCHED_LOCK_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_SCHED_LOCK_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_SCHED_LOCK_ENABLE != 0) && (NOS_CONFIG_SCHED_LOCK_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_SCHED_LOCK_ENABLE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_SLEEP_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_SLEEP_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_SLEEP_ENABLE != 0) && (NOS_CONFIG_SLEEP_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_SLEEP_ENABLE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_SLEEP_UNTIL_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_SLEEP_UNTIL_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_SLEEP_UNTIL_ENABLE != 0) && (NOS_CONFIG_SLEEP_UNTIL_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_SLEEP_UNTIL_ENABLE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_THREAD_SUSPEND_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_THREAD_SUSPEND_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_THREAD_SUSPEND_ENABLE != 0) && (NOS_CONFIG_THREAD_SUSPEND_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_THREAD_SUSPEND_ENABLE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_THREAD_DELETE_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_THREAD_DELETE_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_THREAD_DELETE_ENABLE != 0) && (NOS_CONFIG_THREAD_DELETE_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_THREAD_DELETE_ENABLE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_THREAD_SET_PRIO_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_THREAD_SET_PRIO_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_THREAD_SET_PRIO_ENABLE != 0) && (NOS_CONFIG_THREAD_SET_PRIO_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_THREAD_SET_PRIO_ENABLE is set to invalid value: must be set to 0 or 1."
#endif

#ifndef NOS_CONFIG_SEM_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_SEM_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_SEM_ENABLE != 0) && (NOS_CONFIG_SEM_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_SEM_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_SEM_ENABLE > 0)
 #ifndef NOS_CONFIG_SEM_DELETE_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_SEM_DELETE_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_SEM_DELETE_ENABLE != 0) && (NOS_CONFIG_SEM_DELETE_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_SEM_DELETE_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
 #ifndef NOS_CONFIG_SEM_COUNT_WIDTH
  #error "nOSConfig.h: NOS_CONFIG_SEM_COUNT_WIDTH is not defined: must be set to 8, 16, 32, or 64."
 #elif (NOS_CONFIG_SEM_COUNT_WIDTH != 8) && (NOS_CONFIG_SEM_COUNT_WIDTH != 16) && (NOS_CONFIG_SEM_COUNT_WIDTH != 32) && (NOS_CONFIG_SEM_COUNT_WIDTH != 64)
  #error "nOSConfig.h: NOS_CONFIG_SEM_COUNT_WIDTH is set to invalid value: must be set to 8, 16, 32 or 64."
 #else
  #if (NOS_CONFIG_SEM_COUNT_WIDTH == 8)
   #define NOS_SEM_COUNT_MAX                        UINT8_MAX
  #elif (NOS_CONFIG_SEM_COUNT_WIDTH == 16)
   #define NOS_SEM_COUNT_MAX                        UINT16_MAX
  #elif (NOS_CONFIG_SEM_COUNT_WIDTH == 32)
   #define NOS_SEM_COUNT_MAX                        UINT32_MAX
  #elif (NOS_CONFIG_SEM_COUNT_WIDTH == 64)
   #define NOS_SEM_COUNT_MAX                        UINT64_MAX
  #endif
 #endif
#else
 #undef NOS_CONFIG_SEM_DELETE_ENABLE
 #undef NOS_CONFIG_SEM_COUNT_WIDTH
#endif

#ifndef NOS_CONFIG_MUTEX_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_MUTEX_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_MUTEX_ENABLE != 0) && (NOS_CONFIG_MUTEX_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_MUTEX_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_MUTEX_ENABLE > 0)
 #ifndef NOS_CONFIG_MUTEX_DELETE_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_MUTEX_DELETE_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_MUTEX_DELETE_ENABLE != 0) && (NOS_CONFIG_MUTEX_DELETE_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_MUTEX_DELETE_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
 #ifndef NOS_CONFIG_MUTEX_RECURSIVE_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_MUTEX_RECURSIVE_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE != 0) && (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_MUTEX_RECURSIVE_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
#else
 #undef NOS_CONFIG_MUTEX_DELETE_ENABLE
 #undef NOS_CONFIG_MUTEX_RECURSIVE_ENABLE
#endif

#ifndef NOS_CONFIG_FLAG_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_FLAG_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_FLAG_ENABLE != 0) && (NOS_CONFIG_FLAG_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_FLAG_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_FLAG_ENABLE > 0)
 #ifndef NOS_CONFIG_FLAG_DELETE_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_FLAG_DELETE_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_FLAG_DELETE_ENABLE != 0) && (NOS_CONFIG_FLAG_DELETE_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_FLAG_DELETE_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
 #ifndef NOS_CONFIG_FLAG_NB_BITS
  #error "nOSConfig.h: NOS_CONFIG_FLAG_NB_BITS is not defined: must be set to 8, 16, 32 or 64."
 #elif (NOS_CONFIG_FLAG_NB_BITS != 8) && (NOS_CONFIG_FLAG_NB_BITS != 16) && (NOS_CONFIG_FLAG_NB_BITS != 32) && (NOS_CONFIG_FLAG_NB_BITS != 64)
  #error "nOSConfig.h: NOS_CONFIG_FLAG_NB_BITS is set to invalid value: must be set to 8, 16, 32 or 64."
 #endif
#else
 #undef NOS_CONFIG_FLAG_DELETE_ENABLE
 #undef NOS_CONFIG_FLAG_NB_BITS
#endif

#ifndef NOS_CONFIG_QUEUE_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_QUEUE_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_QUEUE_ENABLE != 0) && (NOS_CONFIG_QUEUE_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_QUEUE_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_QUEUE_ENABLE > 0)
 #ifndef NOS_CONFIG_QUEUE_DELETE_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_QUEUE_DELETE_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_QUEUE_DELETE_ENABLE != 0) && (NOS_CONFIG_QUEUE_DELETE_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_QUEUE_DELETE_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
#else
 #undef NOS_CONFIG_QUEUE_DELETE_ENABLE
#endif

#ifndef NOS_CONFIG_MEM_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_MEM_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_MEM_ENABLE != 0) && (NOS_CONFIG_MEM_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_MEM_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_MEM_ENABLE > 0)
 #ifndef NOS_CONFIG_MEM_DELETE_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_MEM_DELETE_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_MEM_DELETE_ENABLE != 0) && (NOS_CONFIG_MEM_DELETE_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_MEM_DELETE_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
 #ifndef NOS_CONFIG_MEM_SANITY_CHECK_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_MEM_SANITY_CHECK_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE != 0) && (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_MEM_SANITY_CHECK_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
#else
 #undef NOS_CONFIG_MEM_DELETE_ENABLE
 #undef NOS_CONFIG_MEM_SANITY_CHECK_ENABLE
#endif

#ifndef NOS_CONFIG_TIMER_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_TIMER_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_TIMER_ENABLE != 0) && (NOS_CONFIG_TIMER_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_TIMER_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_TIMER_ENABLE > 0)
 #if (NOS_CONFIG_SEM_ENABLE == 0)
  #error "nOSConfig.h: NOS_CONFIG_SEM_ENABLE need to be enable when NOS_CONFIG_TIMER_ENABLE is enable."
 #endif
 #ifndef NOS_CONFIG_TIMER_DELETE_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_TIMER_DELETE_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_TIMER_DELETE_ENABLE != 0) && (NOS_CONFIG_TIMER_DELETE_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_TIMER_DELETE_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
  #ifndef NOS_CONFIG_TIMER_THREAD_PRIO
   #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_PRIO is not defined: must be set between 0 and NOS_CONFIG_HIGHEST_THREAD_PRIO inclusively."
  #elif (NOS_CONFIG_TIMER_THREAD_PRIO < 0)
   #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_PRIO is set to invalid value: must be set between 0 and NOS_CONFIG_HIGHEST_THREAD_PRIO inclusively."
  #elif (NOS_CONFIG_TIMER_THREAD_PRIO > NOS_CONFIG_HIGHEST_THREAD_PRIO)
   #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_PRIO is higher than NOS_CONFIG_HIGHEST_THREAD_PRIO: must be set between 0 and NOS_CONFIG_HIGHEST_THREAD_PRIO inclusively."
  #endif
 #else
  #undef NOS_CONFIG_TIMER_THREAD_PRIO
 #endif
 #ifndef NOS_CONFIG_TIMER_THREAD_STACK_SIZE
  #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_STACK_SIZE is not defined."
 #endif
 #ifndef NOS_CONFIG_TIMER_THREAD_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_TIMER_THREAD_ENABLE != 0) && (NOS_CONFIG_TIMER_THREAD_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_ENABLE is set to invalid value: must be set to 0 or 1."
 #elif (NOS_CONFIG_TIMER_THREAD_ENABLE == 0)
  #undef NOS_CONFIG_TIMER_THREAD_PRIO
  #undef NOS_CONFIG_TIMER_THREAD_STACK_SIZE
 #endif
 #ifndef NOS_CONFIG_TIMER_COUNT_WIDTH
  #error "nOSConfig.h: NOS_CONFIG_TIMER_COUNT_WIDTH is not defined: must be set to 8, 16, 32 or 64."
 #elif (NOS_CONFIG_TIMER_COUNT_WIDTH != 8) && (NOS_CONFIG_TIMER_COUNT_WIDTH != 16) && (NOS_CONFIG_TIMER_COUNT_WIDTH != 32) && (NOS_CONFIG_TIMER_COUNT_WIDTH != 64)
  #error "nOSConfig.h: NOS_CONFIG_TIMER_COUNT_WIDTH is set to invalid value: must be set to 8, 16, 32 or 64."
 #endif
#else
 #undef NOS_CONFIG_TIMER_THREAD_ENABLE
 #undef NOS_CONFIG_TIMER_DELETE_ENABLE
 #undef NOS_CONFIG_TIMER_THREAD_PRIO
 #undef NOS_CONFIG_TIMER_THREAD_STACK_SIZE
 #undef NOS_CONFIG_TIMER_COUNT_WIDTH
#endif

#ifndef NOS_CONFIG_TIME_ENABLE
 #error "nOSConfig.h: NOS_CONFIG_TIME_ENABLE is not defined: must be set to 0 or 1."
#elif (NOS_CONFIG_TIME_ENABLE != 0) && (NOS_CONFIG_TIME_ENABLE != 1)
 #error "nOSConfig.h: NOS_CONFIG_TIME_ENABLE is set to invalid value: must be set to 0 or 1."
#elif (NOS_CONFIG_TIME_ENABLE > 0)
 #ifndef NOS_CONFIG_TIME_WAIT_ENABLE
  #error "nOSConfig.h: NOS_CONFIG_TIME_WAIT_ENABLE is not defined: must be set to 0 or 1."
 #elif (NOS_CONFIG_TIME_WAIT_ENABLE != 0) && (NOS_CONFIG_TIME_WAIT_ENABLE != 1)
  #error "nOSConfig.h: NOS_CONFIG_TIME_WAIT_ENABLE is set to invalid value: must be set to 0 or 1."
 #endif
 #ifndef NOS_CONFIG_TIME_TICKS_PER_SECOND
  #error "nOSConfig.h: NOS_CONFIG_TIME_TICKS_PER_SECOND is not defined: must be higher than 0."
 #elif (NOS_CONFIG_TIME_TICKS_PER_SECOND == 0)
  #error "nOSConfig.h: NOS_CONFIG_TIME_TICKS_PER_SECOND is set to invalue value: must be higher than 0."
 #endif
 #ifndef NOS_CONFIG_TIME_COUNT_WIDTH
  #error "nOSConfig.h: NOS_CONFIG_TIME_COUNT_WIDTH is not defined: must be set to 32 or 64."
 #elif (NOS_CONFIG_TIME_COUNT_WIDTH != 32) && (NOS_CONFIG_TIME_COUNT_WIDTH != 64)
  #error "nOSConfig.h: NOS_CONFIG_TIME_COUNT_WIDTH is set to invalid value: must be set to 32 or 64."
 #endif
#else
 #undef NOS_CONFIG_TIME_WAIT_ENABLE
 #undef NOS_CONFIG_TIME_TICKS_PER_SECOND
 #undef NOS_CONFIG_TIME_COUNT_WIDTH
#endif

typedef struct _nOS_List            nOS_List;
typedef struct _nOS_Node            nOS_Node;
typedef void(*nOS_NodeHandler)(void*,void*);
typedef struct _nOS_Thread          nOS_Thread;
typedef void(*nOS_ThreadEntry)(void*);
typedef struct _nOS_Event           nOS_Event;
#if (NOS_CONFIG_TICK_COUNT_WIDTH == 8)
 typedef uint8_t                    nOS_TickCounter;
#elif (NOS_CONFIG_TICK_COUNT_WIDTH == 16)
 typedef uint16_t                   nOS_TickCounter;
#elif (NOS_CONFIG_TICK_COUNT_WIDTH == 32)
 typedef uint32_t                   nOS_TickCounter;
#elif (NOS_CONFIG_TICK_COUNT_WIDTH == 64)
 typedef uint64_t                   nOS_TickCounter;
#endif
#if (NOS_CONFIG_SEM_ENABLE > 0)
 typedef struct _nOS_Sem            nOS_Sem;
 #if (NOS_CONFIG_SEM_COUNT_WIDTH == 8)
  typedef uint8_t                   nOS_SemCounter;
 #elif (NOS_CONFIG_SEM_COUNT_WIDTH == 16)
  typedef uint16_t                  nOS_SemCounter;
 #elif (NOS_CONFIG_SEM_COUNT_WIDTH == 32)
  typedef uint32_t                  nOS_SemCounter;
 #elif (NOS_CONFIG_SEM_COUNT_WIDTH == 64)
  typedef uint64_t                  nOS_SemCounter;
 #endif
#endif
#if (NOS_CONFIG_MUTEX_ENABLE > 0)
 typedef struct _nOS_Mutex          nOS_Mutex;
#endif
#if (NOS_CONFIG_QUEUE_ENABLE > 0)
 typedef struct _nOS_Queue          nOS_Queue;
#endif
#if (NOS_CONFIG_FLAG_ENABLE > 0)
 typedef struct _nOS_Flag           nOS_Flag;
 typedef struct _nOS_FlagContext    nOS_FlagContext;
 typedef struct _nOS_FlagResult     nOS_FlagResult;
 #if (NOS_CONFIG_FLAG_NB_BITS == 8)
  typedef uint8_t                   nOS_FlagBits;
 #elif (NOS_CONFIG_FLAG_NB_BITS == 16)
  typedef uint16_t                  nOS_FlagBits;
 #elif (NOS_CONFIG_FLAG_NB_BITS == 32)
  typedef uint32_t                  nOS_FlagBits;
 #elif (NOS_CONFIG_FLAG_NB_BITS == 64)
  typedef uint64_t                  nOS_FlagBits;
 #endif
#endif
#if (NOS_CONFIG_MEM_ENABLE > 0)
 typedef struct _nOS_Mem            nOS_Mem;
#endif
#if (NOS_CONFIG_TIMER_ENABLE > 0)
 typedef struct _nOS_Timer          nOS_Timer;
 #if (NOS_CONFIG_TIMER_COUNT_WIDTH == 8)
  typedef uint8_t                   nOS_TimerCounter;
 #elif (NOS_CONFIG_TIMER_COUNT_WIDTH == 16)
  typedef uint16_t                  nOS_TimerCounter;
 #elif (NOS_CONFIG_TIMER_COUNT_WIDTH == 32)
  typedef uint32_t                  nOS_TimerCounter;
 #elif (NOS_CONFIG_TIMER_COUNT_WIDTH == 64)
  typedef uint64_t                  nOS_TimerCounter;
 #endif
 typedef void(*nOS_TimerCallback)(nOS_Timer*,void*);
#endif
#if (NOS_CONFIG_TIME_ENABLE > 0)
 #if (NOS_CONFIG_TIME_COUNT_WIDTH == 32)
  typedef uint32_t                  nOS_Time;
 #elif (NOS_CONFIG_TIME_COUNT_WIDTH == 64)
  typedef uint64_t                  nOS_Time;
 #endif
 typedef struct _nOS_TimeDate       nOS_TimeDate;
 typedef struct _nOS_TimeContext    nOS_TimeContext;
#endif

typedef enum _nOS_Error
{
    NOS_OK = 0,
    NOS_E_NULL = -1,
    NOS_E_INV_VAL = -2,
    NOS_E_LOCKED = -3,
    NOS_E_ISR = -4,
    NOS_E_IDLE = -5,
    NOS_E_TIMEOUT = -6,
    NOS_E_UNDERFLOW = -7,
    NOS_E_OVERFLOW = -8,
    NOS_E_AGAIN = -9,
    NOS_E_OWNER = -10,
    NOS_E_EMPTY = -11,
    NOS_E_FULL = -12,
    NOS_E_INIT = -13,
    NOS_E_DELETED = -14,
    NOS_E_INV_OBJ = -15,
    NOS_E_ELAPSED = -16,
    NOS_E_NOT_CREATED = -17,
    NOS_E_INV_STATE = -18
} nOS_Error;

#include "nOSPort.h"

/* Port specific config checkup */
#ifdef NOS_PORT_SEPARATE_CALL_STACK
 #if (NOS_CONFIG_TIMER_ENABLE > 0)
  #if (NOS_CONFIG_TIMER_THREAD_ENABLE > 0)
   #ifndef NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE
    #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE is not defined: must be higher than 0."
   #elif (NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE == 0)
    #error "nOSConfig.h: NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE is set to invalid value: must be higher than 0."
   #endif
  #else
   #undef NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE
  #endif
 #else
  #undef NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE
 #endif
#else
 #undef NOS_CONFIG_TIMER_THREAD_CALL_STACK_SIZE
#endif

#ifdef NOS_PORT_NO_CONST
 #define NOS_CONST
#else
 #define NOS_CONST                  const
#endif

struct _nOS_List
{
    nOS_Node            *head;
    nOS_Node            *tail;
};

struct _nOS_Node
{
    nOS_Node            *prev;
    nOS_Node            *next;
    void                *payload;
};

struct _nOS_Thread
{
    nOS_Stack           *stackPtr;
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
    uint8_t             prio;
#endif
    nOS_Error           error;
    uint8_t             state;
    nOS_TickCounter     timeout;
    nOS_Event           *event;
    void                *context;

    nOS_Node            main;
    nOS_Node            readyWait;
};

struct _nOS_Event
{
#if (NOS_CONFIG_SAFE > 0)
    uint8_t             type;
#endif
    nOS_List            waitList;
};

#if (NOS_CONFIG_SEM_ENABLE > 0)
struct _nOS_Sem
{
    nOS_Event           e;
    nOS_SemCounter      count;
    nOS_SemCounter      max;
};
#endif

#if (NOS_CONFIG_MUTEX_ENABLE > 0)
struct _nOS_Mutex
{
    nOS_Event           e;
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
    uint8_t             prio;
    uint8_t             backup;
 #endif
    nOS_Thread          *owner;
 #if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
    uint8_t             type;
    uint8_t             count;
 #endif
};
#endif

#if (NOS_CONFIG_QUEUE_ENABLE > 0)
struct _nOS_Queue
{
    nOS_Event           e;
    uint8_t             *buffer;
    uint16_t            bsize;
    uint16_t            bmax;
    uint16_t            bcount;
    uint16_t            r;
    uint16_t            w;
};
#endif

#if (NOS_CONFIG_FLAG_ENABLE > 0)
struct _nOS_Flag
{
    nOS_Event           e;
    nOS_FlagBits        flags;
};

struct _nOS_FlagContext
{
    uint8_t             opt;
    nOS_FlagBits        flags;
    nOS_FlagBits        *rflags;
};

struct _nOS_FlagResult
{
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
    bool                sched;
 #endif
    nOS_FlagBits        rflags;
};
#endif

#if (NOS_CONFIG_MEM_ENABLE > 0)
struct _nOS_Mem
{
    nOS_Event           e;
    void                **blist;
 #if (NOS_CONFIG_MEM_SANITY_CHECK_ENABLE > 0)
    uint8_t             *buffer;
    uint16_t            bsize;
    uint16_t            bcount;
    uint16_t            bmax;
 #endif
};
#endif

#if (NOS_CONFIG_TIMER_ENABLE > 0)
struct _nOS_Timer
{
    uint8_t             state;
    nOS_TimerCounter    count;
    nOS_TimerCounter    reload;
    nOS_TimerCallback   callback;
    void                *arg;
    nOS_Node            node;
};
#endif

#if (NOS_CONFIG_TIME_ENABLE > 0)
struct _nOS_TimeDate
{
    uint16_t            year;           /* From 1970 to ... */
    uint8_t             month;          /* From 1 (January) to 12 (December) */
    uint8_t             day;            /* From 1 to 31 */
    uint8_t             weekday;        /* From 1 (Monday) to 7 (Sunday) */
    uint8_t             hour;           /* From 0 to 23 */
    uint8_t             minute;         /* From 0 to 59 */
    uint8_t             second;         /* From 0 to 59 */
};
#endif

#define NOS_NO_WAIT                 0
#if (NOS_CONFIG_TICK_COUNT_WIDTH == 8)
 #define NOS_WAIT_INFINITE          UINT8_MAX
#elif (NOS_CONFIG_TICK_COUNT_WIDTH == 16)
 #define NOS_WAIT_INFINITE          UINT16_MAX
#elif (NOS_CONFIG_TICK_COUNT_WIDTH == 32)
 #define NOS_WAIT_INFINITE          UINT32_MAX
#elif (NOS_CONFIG_TICK_COUNT_WIDTH == 64)
 #define NOS_WAIT_INFINITE          UINT64_MAX
#endif

#define NOS_THREAD_PRIO_IDLE        0

#define NOS_THREAD_STOPPED          0x00
#define NOS_THREAD_TAKING_SEM       0x01
#define NOS_THREAD_LOCKING_MUTEX    0x02
#define NOS_THREAD_READING_QUEUE    0x03
#define NOS_THREAD_WRITING_QUEUE    0x04
#define NOS_THREAD_WAITING_FLAG     0x05
#define NOS_THREAD_ALLOC_MEM        0x06
#define NOS_THREAD_WAITING_EVENT    0x07
#define NOS_THREAD_SLEEPING         0x08
#define NOS_THREAD_SLEEPING_UNTIL   0x10
#define NOS_THREAD_WAITING_TIME     0x20
#define NOS_THREAD_SUSPENDED        0x40
#define NOS_THREAD_READY            0x80

#define NOS_EVENT_INVALID           0x00
#define NOS_EVENT_BASE              0x01
#define NOS_EVENT_SEM               0x02
#define NOS_EVENT_MUTEX             0x03
#define NOS_EVENT_QUEUE             0x04
#define NOS_EVENT_FLAG              0x05
#define NOS_EVENT_MEM               0x06

#define NOS_MUTEX_NORMAL            0
#define NOS_MUTEX_RECURSIVE         1
#define NOS_MUTEX_PRIO_INHERIT      0

#define NOS_FLAG_WAIT               0x01
#define NOS_FLAG_WAIT_ANY           0x00
#define NOS_FLAG_WAIT_ALL           0x01
#define NOS_FLAG_CLEAR_ON_EXIT      0x02
#define NOS_FLAG_NONE               0
#define NOS_FLAG_TEST_ANY           false
#define NOS_FLAG_TEST_ALL           true

#define NOS_TIMER_DELETED           0x00
#define NOS_TIMER_ONE_SHOT          0x00
#define NOS_TIMER_FREE_RUNNING      0x01
#define NOS_TIMER_MODE              0x01
#define NOS_TIMER_PAUSED            0x20
#define NOS_TIMER_RUNNING           0x40
#define NOS_TIMER_CREATED           0x80

#ifdef NOS_PRIVATE
 NOS_EXTERN nOS_Thread      nOS_idleHandle;
 NOS_EXTERN nOS_TickCounter nOS_tickCounter;
 NOS_EXTERN uint8_t         nOS_isrNestingCounter;
 #if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
  NOS_EXTERN uint8_t        nOS_lockNestingCounter;
 #endif

 NOS_EXTERN nOS_Thread      *nOS_runningThread;
 NOS_EXTERN nOS_Thread      *nOS_highPrioThread;
 NOS_EXTERN nOS_List        nOS_mainList;
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
  NOS_EXTERN nOS_List       nOS_readyList[NOS_CONFIG_HIGHEST_THREAD_PRIO+1];
 #else
  NOS_EXTERN nOS_List       nOS_readyList;
 #endif
#endif

#ifdef NOS_PRIVATE
 nOS_Thread*    SchedHighPrio               (void);
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
  void          AppendThreadToReadyList     (nOS_Thread *thread);
  void          RemoveThreadFromReadyList   (nOS_Thread *thread);
 #endif
 #if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
  void          DeleteThread                (void *payload, void *arg);
  void          SuspendThread               (void *payload, void *arg);
  void          ResumeThread                (void *payload, void *arg);
 #endif
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
  void          ChangeThreadPrio           (nOS_Thread *thread, uint8_t prio);
 #endif
 void           TickThread                  (void *payload, void *arg);
 void           SignalThread                (nOS_Thread *thread, nOS_Error err);
#endif

nOS_Error       nOS_Init                    (void);
nOS_Error       nOS_Sched                   (void);
nOS_Error       nOS_Yield                   (void);
void            nOS_Tick                    (void);
nOS_TickCounter nOS_TickCount               (void);
#if (NOS_CONFIG_SLEEP_ENABLE > 0)
 nOS_Error      nOS_Sleep                   (nOS_TickCounter ticks);
#endif
#if (NOS_CONFIG_SLEEP_UNTIL_ENABLE > 0)
 nOS_Error      nOS_SleepUntil              (nOS_TickCounter tick);
#endif

#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
 nOS_Error      nOS_SchedLock               (void);
 nOS_Error      nOS_SchedUnlock             (void);
#endif

void            nOS_ListInit                (nOS_List *list);
void*           nOS_ListHead                (nOS_List *list);
void            nOS_ListAppend              (nOS_List *list, nOS_Node *node);
void            nOS_ListRemove              (nOS_List *list, nOS_Node *node);
void            nOS_ListRotate              (nOS_List *list);
void            nOS_ListWalk                (nOS_List *list, nOS_NodeHandler handler, void *arg);



nOS_Error       nOS_ThreadCreate            (nOS_Thread *thread, nOS_ThreadEntry entry, void *arg, nOS_Stack *stack, size_t ssize
#ifdef NOS_PORT_SEPARATE_CALL_STACK
                                             ,size_t cssize
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                                             ,uint8_t prio
#endif
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
                                             ,uint8_t state
#endif
                                             );
#if (NOS_CONFIG_THREAD_DELETE_ENABLE > 0)
 nOS_Error      nOS_ThreadDelete            (nOS_Thread *thread);
#endif
void            nOS_ThreadTick              (void);
#if (NOS_CONFIG_THREAD_SUSPEND_ENABLE > 0)
 nOS_Error      nOS_ThreadSuspend           (nOS_Thread *thread);
 nOS_Error      nOS_ThreadSuspendAll        (void);
 nOS_Error      nOS_ThreadResume            (nOS_Thread *thread);
 nOS_Error      nOS_ThreadResumeAll         (void);
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_THREAD_SET_PRIO_ENABLE > 0)
 int16_t        nOS_ThreadGetPriority       (nOS_Thread *thread);
 nOS_Error      nOS_ThreadSetPriority       (nOS_Thread *thread, uint8_t prio);
#endif
nOS_Thread*     nOS_ThreadRunning           (void);

#if (NOS_CONFIG_SAFE > 0)
 void           nOS_EventCreate             (nOS_Event *event, uint8_t type);
#else
 void           nOS_EventCreate             (nOS_Event *event);
#endif
#if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0) && (NOS_CONFIG_SCHED_PREEMPTIVE_ENABLE > 0)
 bool           nOS_EventDelete             (nOS_Event *event);
#else
 void           nOS_EventDelete             (nOS_Event *event);
#endif
nOS_Error       nOS_EventWait               (nOS_Event *event, uint8_t state, nOS_TickCounter tout);
nOS_Thread*     nOS_EventSignal             (nOS_Event *event, nOS_Error err);

#if (NOS_CONFIG_SEM_ENABLE > 0)
 nOS_Error      nOS_SemCreate               (nOS_Sem *sem, nOS_SemCounter cntr, nOS_SemCounter max);
 #if (NOS_CONFIG_SEM_DELETE_ENABLE > 0)
  nOS_Error     nOS_SemDelete               (nOS_Sem *sem);
 #endif
 nOS_Error      nOS_SemTake                 (nOS_Sem *sem, nOS_TickCounter tout);
 nOS_Error      nOS_SemGive                 (nOS_Sem *sem);
 bool           nOS_SemIsAvailable          (nOS_Sem *sem);
#endif

#if (NOS_CONFIG_MUTEX_ENABLE > 0)
 nOS_Error      nOS_MutexCreate             (nOS_Mutex *mutex
 #if (NOS_CONFIG_HIGHEST_THREAD_PRIO > 0)
                                             ,uint8_t prio
 #endif
 #if (NOS_CONFIG_MUTEX_RECURSIVE_ENABLE > 0)
                                             ,uint8_t type
 #endif
                                             );
 #if (NOS_CONFIG_MUTEX_DELETE_ENABLE > 0)
  nOS_Error     nOS_MutexDelete             (nOS_Mutex *mutex);
 #endif
 nOS_Error      nOS_MutexLock               (nOS_Mutex *mutex, nOS_TickCounter tout);
 nOS_Error      nOS_MutexUnlock             (nOS_Mutex *mutex);
 bool           nOS_MutexIsLocked           (nOS_Mutex *mutex);
 nOS_Thread*    nOS_MutexOwner              (nOS_Mutex *mutex);
#endif

#if (NOS_CONFIG_QUEUE_ENABLE > 0)
 nOS_Error      nOS_QueueCreate             (nOS_Queue *queue, void *buffer, uint16_t bsize, uint16_t bmax);
 #if (NOS_CONFIG_QUEUE_DELETE_ENABLE > 0)
  nOS_Error     nOS_QueueDelete             (nOS_Queue *queue);
 #endif
 nOS_Error      nOS_QueueRead               (nOS_Queue *queue, void *buffer, nOS_TickCounter tout);
 nOS_Error      nOS_QueueWrite              (nOS_Queue *queue, void *buffer, nOS_TickCounter tout);
 bool           nOS_QueueIsEmpty            (nOS_Queue *queue);
 bool           nOS_QueueIsFull             (nOS_Queue *queue);
#endif

#if (NOS_CONFIG_FLAG_ENABLE > 0)
 nOS_Error      nOS_FlagCreate              (nOS_Flag *flag, nOS_FlagBits flags);
 #if (NOS_CONFIG_FLAG_DELETE_ENABLE > 0)
  nOS_Error     nOS_FlagDelete              (nOS_Flag *flag);
 #endif
 nOS_Error      nOS_FlagWait                (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits *res, uint8_t opt, nOS_TickCounter tout);
 nOS_Error      nOS_FlagSend                (nOS_Flag *flag, nOS_FlagBits flags, nOS_FlagBits mask);
 nOS_FlagBits   nOS_FlagTest                (nOS_Flag *flag, nOS_FlagBits flags, bool all);
#endif

#if (NOS_CONFIG_MEM_ENABLE > 0)
 nOS_Error      nOS_MemCreate               (nOS_Mem *mem, void *buffer, size_t bsize, uint16_t max);
 #if (NOS_CONFIG_MEM_DELETE_ENABLE > 0)
  nOS_Error     nOS_MemDelete               (nOS_Mem *mem);
 #endif
 void*          nOS_MemAlloc                (nOS_Mem *mem, nOS_TickCounter tout);
 nOS_Error      nOS_MemFree                 (nOS_Mem *mem, void *block);
 bool           nOS_MemIsAvailable          (nOS_Mem *mem);
#endif

#if (NOS_CONFIG_TIMER_ENABLE > 0)
 #ifdef NOS_PRIVATE
  void          nOS_TimerInit               (void);
 #endif
 void           nOS_TimerTick               (void);
 void           nOS_TimerProcess            (void);
 nOS_Error      nOS_TimerCreate             (nOS_Timer *timer, nOS_TimerCallback callback, void *arg, nOS_TimerCounter reload, uint8_t opt);
 #if (NOS_CONFIG_TIMER_DELETE_ENABLE > 0)
  nOS_Error     nOS_TimerDelete             (nOS_Timer *timer);
 #endif
 nOS_Error      nOS_TimerStart              (nOS_Timer *timer);
 nOS_Error      nOS_TimerStop               (nOS_Timer *timer);
 nOS_Error      nOS_TimerRestart            (nOS_Timer *timer, nOS_TimerCounter reload);
 nOS_Error      nOS_TimerPause              (nOS_Timer *timer);
 nOS_Error      nOS_TimerResume             (nOS_Timer *timer);
 nOS_Error      nOS_TimerChangeReload       (nOS_Timer *timer, nOS_TimerCounter reload);
 nOS_Error      nOS_TimerChangeCallback     (nOS_Timer *timer, nOS_TimerCallback callback, void *arg);
 nOS_Error      nOS_TimerChangeMode         (nOS_Timer *timer, uint8_t opt);
 bool           nOS_TimerIsRunning          (nOS_Timer *timer);
#endif

#if (NOS_CONFIG_TIME_ENABLE > 0)
 #ifdef NOS_PRIVATE
  void          nOS_TimeInit                (void);
 #endif
 void           nOS_TimeTick                (void);
 nOS_Time       nOS_TimeNow                 (void);
 nOS_Error      nOS_TimeChange              (nOS_Time time);
 nOS_TimeDate   nOS_TimeConvert             (nOS_Time time);
 #if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
  nOS_Error     nOS_TimeWait                (nOS_Time time);
 #endif
 nOS_TimeDate   nOS_TimeDateNow             (void);
 nOS_Error      nOS_TimeDateChange          (nOS_TimeDate *timedate);
 nOS_Time       nOS_TimeDateConvert         (nOS_TimeDate *timedate);
 #if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
  nOS_Error     nOS_TimeDateWait            (nOS_TimeDate *timedate);
 #endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* NOS_H */
