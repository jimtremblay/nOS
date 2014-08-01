/*
 * Define to value other than 0 to enable arguments checking in nOS API.
 */
#define NOS_CONFIG_SAFE                         0

/*
 * Highest priority a thread can take (up to 255).
 */
#define NOS_CONFIG_MAX_THREAD_PRIO              31

/*
 * Stack size to use from interrupt routines in number of stack_t entries.
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

/*
 *
 */
#define NOS_CONFIG_TIMER_EN
#define NOS_CONFIG_TIMER_THREAD_PRIO            1
#define NOS_CONFIG_TIMER_THREAD_STACK_SIZE      128
