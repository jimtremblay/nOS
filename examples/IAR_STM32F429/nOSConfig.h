/* Define to value other than 0 to enable arguments checking in nOS API. */
#define NOS_CONFIG_SAFE                         0

/* Highest priority a thread can take (up to 255). */
#define NOS_CONFIG_MAX_THREAD_PRIO              31

/* Enable or disable scheduler locking. */
#define NOS_CONFIG_SCHED_LOCK_ENABLE            1
/* Enable or disable sleeping from running thread. */
#define NOS_CONFIG_SLEEP_ENABLE                 1

/* Enable or disable thread resume/suspend at run-time. */
#define NOS_CONFIG_THREAD_SUSPEND_ENABLE        1
/* Enable or disable thread set/get priority at run-time. */
#define NOS_CONFIG_THREAD_SET_PRIO_ENABLE       1

/* Enable of disable semaphore. */
#define NOS_CONFIG_SEM_ENABLE                   1

/* Enable of disable mutex. */
#define NOS_CONFIG_MUTEX_ENABLE                 1

/* Enable or disable flag. */
#define NOS_CONFIG_FLAG_ENABLE                  1
/* Size of flag in bits (can be 8, 16 or 32) */
#define NOS_CONFIG_FLAG_NB_BITS                 32

/* Enable or disable queue. */
#define NOS_CONFIG_QUEUE_ENABLE                 1

/* Enable or disable mem. */
#define NOS_CONFIG_MEM_ENABLE                   1

/* Enable or disable timer thread with callback. */
#define NOS_CONFIG_TIMER_ENABLE                 1
/* Priority of timer thread. */
#define NOS_CONFIG_TIMER_THREAD_PRIO            1
/* Stack size of timer thread. */
#define NOS_CONFIG_TIMER_THREAD_STACK_SIZE      128

/*
 * Stack size to use from interrupt routines in number of nOS_Stack entries.
 * Not used on all platforms.
 */
#define NOS_CONFIG_ISR_STACK_SIZE               128

/*
 * Highest priority of interrupt routines that use nOS API which can enable
 * zero interrupt latency for high priority ISR. You should not call any nOS
 * API from interrupt handlers with priority higher than this setting. Can't
 * be set to zero.
 * Lower number = Higher priority
 * Not used on all platforms.
 */
#define NOS_CONFIG_MAX_UNSAFE_ISR_PRIO          5

