/*
 * Copyright (c) 2014-2015 Jim Tremblay
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define NOS_PRIVATE
#include "nOS.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if (NOS_CONFIG_TIME_ENABLE > 0)

#define DAYS_PER_YEAR(y)            (IsLeapYear(y)?366:365)
#define DAYS_PER_MONTH(m,y)         ((IsLeapYear(y)&&((m)==2))?29:daysPerMonth[(m)-1])

#define MAX_TIME_SLEEP              ((NOS_WAIT_INFINITE-1)/NOS_CONFIG_TIME_TICKS_PER_SECOND)
#define MAX_TICKS_SLEEP             (MAX_TIME_SLEEP*NOS_CONFIG_TIME_TICKS_PER_SECOND)

static uint16_t         timePrescaler;
static nOS_Time         timeCounter;
#if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
static nOS_Event        timeEvent;
#endif

static const uint8_t    daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static inline bool IsLeapYear (uint16_t year)
{
    return ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0));
}

#if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
static void TickTime(void *payload, void *arg)
{
    nOS_Thread      *thread = (nOS_Thread*)payload;
    nOS_TimeContext *ctx    = (nOS_TimeContext*)thread->context;

    /* Avoid warning */
    NOS_UNUSED(arg);

    if (timeCounter == ctx->time) {
        SignalThread(thread, NOS_OK);
    }
}
#endif

void nOS_TimeInit (void)
{
    timePrescaler = 0;
    timeCounter = 0;
#if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
#if (NOS_CONFIG_SAFE > 0)
    nOS_EventCreate(&timeEvent, NOS_EVENT_SLEEP);
#else
    nOS_EventCreate(&timeEvent);
#endif
#endif
}

void nOS_TimeTick (void)
{
    nOS_CriticalEnter();
    timePrescaler++;
    timePrescaler %= NOS_CONFIG_TIME_TICKS_PER_SECOND;
    if (timePrescaler == 0) {
        timeCounter++;
#if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
        nOS_ListWalk(&timeEvent.waitingList, TickTime, NULL);
#endif
    }
    nOS_CriticalLeave();
}

nOS_Time nOS_TimeNow (void)
{
    nOS_Time    time;

    nOS_CriticalEnter();
    time = timeCounter;
    nOS_CriticalLeave();

    return time;
}

void nOS_TimeChange (nOS_Time time)
{
    nOS_CriticalEnter();
    timeCounter = time;
    timePrescaler = 0;
    nOS_CriticalLeave();
}

nOS_TimeDate nOS_TimeConvert (nOS_Time time)
{
    nOS_TimeDate    timedate;
    uint16_t        days;

    /* First extract HH:MM:SS from timeCounter */
    timedate.second = time % 60;
    time /= 60;
    timedate.minute = time % 60;
    time /= 60;
    timedate.hour = time % 24;
    time /= 24;
    /* At this point, time is now in number of days since 1st January 1970 */

    /* Second, get week day we are */
    /* 1st January 1970 was a Thursday (4th day or index 3) */
    /* weekday go from 1 (Monday) to 7 (Sunday) */
    timedate.weekday = ((time + 3) % 7) + 1;

    /* Third, find in which year we are */
    timedate.year = 1970;
    days = 365; /* 1970 is not a leap year */
    while (time >= days) {
        time -= days;
        timedate.year++;
        days = DAYS_PER_YEAR(timedate.year);
    }

    /* Fourth, find in which month of the present year we are */
    timedate.month = 1;
    days = 31;  /* January have 31 days */
    while (time >= days) {
        time -= days;
        timedate.month++;
        days = DAYS_PER_MONTH(timedate.month, timedate.year);
    }

    /* Last, we have in which day of month we are */
    timedate.day = time + 1;

    return timedate;
}

#if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
nOS_Error nOS_TimeWait (nOS_Time time)
{
    nOS_Error       err;
    nOS_TimeContext ctx;

    if (nOS_isrNestingCounter > 0) {
        err = NOS_E_ISR;
    }
#if (NOS_CONFIG_SCHED_LOCK_ENABLE > 0)
    /* Can't switch context when scheduler is locked */
    else if (nOS_lockNestingCounter > 0) {
        err = NOS_E_LOCKED;
    }
#endif
    else if (nOS_runningThread == &nOS_mainThread) {
        err = NOS_E_IDLE;
    } else {
        nOS_CriticalEnter();
        if (timeCounter < time) {
            err = NOS_E_ELAPSED;
        } else if (timeCounter == time) {
            err = NOS_OK;
        } else {
            ctx.time = time;
            nOS_runningThread->context = &ctx;
            err = nOS_EventWait(&timeEvent, NOS_THREAD_SLEEPING, NOS_WAIT_INFINITE);
        }
        nOS_CriticalLeave();
    }

    return err;
}
#endif

nOS_TimeDate nOS_TimeDateNow (void)
{
    return nOS_TimeConvert(nOS_TimeNow());
}

void nOS_TimeDateChange (nOS_TimeDate *timedate)
{
#if (NOS_CONFIG_SAFE > 0)
    if (timedate != NULL)
#endif
    {
        nOS_TimeChange(nOS_TimeDateConvert(timedate));
    }
}

nOS_Time nOS_TimeDateConvert (nOS_TimeDate *timedate)
{
    nOS_Time    time = 0;

#if (NOS_CONFIG_SAFE > 0)
    if (timedate != NULL)
#endif
    {
        uint16_t    year;
        uint8_t     month;

        /* Increment time variable until we reach timedate given by user */

        /* Do not count half day */
        time += ((timedate->day-1) * 86400UL); /* 86400 seconds per day */

        /* Do not count on-going month */
        month = 1;
        while (month < timedate->month) {
            time += (DAYS_PER_MONTH(month, timedate->year) * 86400UL);
            month++;
        }

        /* Do not count on-going year */
        year = 1970;
        while (year < timedate->year) {
            time += (DAYS_PER_YEAR(year) * 86400UL);
            year++;
        }

        time += (timedate->hour * 3600UL);
        time += (timedate->minute * 60UL);
        time += timedate->second;
    }

    return time;
}

#if (NOS_CONFIG_TIME_WAIT_ENABLE > 0)
nOS_Error nOS_TimeDateWait (nOS_TimeDate *timedate)
{
    nOS_Error   err;

#if (NOS_CONFIG_SAFE > 0)
    if (timedate == NULL) {
        err = NOS_E_NULL;
    } else
#endif
    {
        err = nOS_TimeWait(nOS_TimeDateConvert(timedate));
    }

    return err;
}
#endif
#endif  /* NOS_CONFIG_TIME_ENABLE */

#if defined(__cplusplus)
}
#endif
