/*
 * Define to value other than 0 to enable arguments checking in nOS API.
 */
#define NOS_CONFIG_SAFE                         0

/*
 * Enable or disable thread resume/suspend at run-time.
 */
#define NOS_CONFIG_THREAD_SUSPEND_ENABLE        0

/*
 * Highest priority a thread can take (up to 255).
 */
#define NOS_CONFIG_MAX_THREAD_PRIO              7

#define NOS_CONFIG_TIMER_ENABLE                 0
#define NOS_CONFIG_TIMER_THREAD_PRIO            1
#define NOS_CONFIG_TIMER_THREAD_STACK_SIZE      128