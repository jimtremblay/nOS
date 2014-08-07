/* Define to value other than 0 to enable arguments checking in nOS API. */
#define NOS_CONFIG_SAFE                         0

/* Highest priority a thread can take (up to 255). */
#define NOS_CONFIG_MAX_THREAD_PRIO              7

/* Enable or disable thread resume/suspend. */
#define NOS_CONFIG_THREAD_SUSPEND_ENABLE        0

/* Enable of disable semaphore. */
#define NOS_CONFIG_SEM_ENABLE                   1
/* Enable or disable dynamic creation of semaphore. */
#define NOS_CONFIG_SEM_CREATE_ENABLE            0

/* Enable or disable timer thread with callback. */
#define NOS_CONFIG_TIMER_ENABLE                 0
/* Priority of timer thread. */
#define NOS_CONFIG_TIMER_THREAD_PRIO            1
/* Stack size of timer thread. */
#define NOS_CONFIG_TIMER_THREAD_STACK_SIZE      128