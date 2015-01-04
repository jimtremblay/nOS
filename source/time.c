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

static uint16_t timePrescaler;
static nOS_Time timeCounter;

static const uint8_t daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static inline bool IsLeapYear (uint16_t year)
{
    return ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0));
}

void nOS_TimeInit (void)
{
    timePrescaler = 0;
    timeCounter = 0;
}

void nOS_TimeTick (void)
{
    nOS_CriticalEnter();
    timePrescaler++;
    timePrescaler %= NOS_CONFIG_TIME_TICKS_PER_SECOND;
    if (timePrescaler == 0) {
        timeCounter++;
    }
    nOS_CriticalLeave();
}

nOS_Time nOS_TimeGet (void)
{
    nOS_Time    time;

    nOS_CriticalEnter();
    time = timeCounter;
    nOS_CriticalLeave();

    return time;
}

void nOS_TimeSet (nOS_Time time)
{
    nOS_CriticalEnter();
    timeCounter = time;
    timePrescaler = 0;
    nOS_CriticalLeave();
}

nOS_TimeDate nOS_TimeDateMake (nOS_Time time)
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

nOS_Time nOS_TimeMake (nOS_TimeDate *timedate)
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

nOS_TimeDate nOS_TimeDateGet (void)
{
    return nOS_TimeDateMake(nOS_TimeGet());
}

void nOS_TimeDateSet (nOS_TimeDate *timedate)
{
#if (NOS_CONFIG_SAFE > 0)
    if (timedate != NULL)
#endif
    {
        nOS_TimeSet(nOS_TimeMake(timedate));
    }
}
#endif  /* NOS_CONFIG_TIME_ENABLE */

#if defined(__cplusplus)
}
#endif
