/* Copyright (c) 1993 by Sanjay Ghemawat */

#include <stddef.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>
#include <alloca.h>
#include <stdlib.h>

#include "config.h"

#include "Month.h"
#include "WeekDay.h"
#include "Time_.h"

/*
 * Want to get high resolution for region of time that will be most
 * heavily used.  This region will probably be around now. Therefore,
 * rep will be number of seconds elapsed since we construct the first
 * Time value for this run of the program.
 */

int    Time::initialized = 0;   /* Initialized yet? */
double Time::offset = 0.0;      /* Offset for time values */
int    Time::junkInt = 0;

void Time::Initialize() {
    struct timeval buf;
    gettimeofday(&buf, 0);

    offset = buf.tv_sec;
    initialized = 1;
}

Time Time::Now() {
    struct timeval buf;
    gettimeofday(&buf, 0);
    return Time(buf);
}

void Time::BreakDownDate(int& mday,WeekDay& wday,Month& month, int& year) const
{
    this->BreakDown(mday, wday, month, year);
}

void Time::BreakDownClock(int& hour, int& min, int& sec, int& milli) const {
    WeekDay junkWDay;
    Month   junkMonth;

    BreakDown(junkInt,junkWDay,junkMonth,junkInt, hour, min, sec, milli);
}

void Time::BreakDown(int& mday, WeekDay& wday, Month& month, int& year,
                     int& hour, int& min, int& sec, int& milli,
                     const char *tz) const
{
    char *old;
    if (! initialized) Initialize();

    time_t clock = (time_t) round(rep + offset);

    if (tz) {
        if ((old=getenv("TZ"))) old=strdup(old);
        setenv("TZ", tz, 1);
        tzset();
    }

    struct tm* t = localtime(&clock);

    if (tz) {
        if (old) setenv("TZ", old, 1); else unsetenv("TZ");
        tzset();
        free(old);
    }

    mday  = t->tm_mday;                         /* tm_mday in 1..31 */
    wday  = WeekDay::Sunday() + t->tm_wday;     /* tm_wday in 0..6.  Sun = 0 */
    month = Month::January() + t->tm_mon;       /* tm_mon  in 0..11. Jan = 0 */
    year  = t->tm_year + 1900;
    hour  = t->tm_hour;
    min   = t->tm_min;
    sec   = t->tm_sec;
    milli = (int)round((rep - floor(rep)) * 1000);
}

Time::Time(const struct timeval& tv) {
    if (! initialized) Initialize();
    rep = (tv.tv_sec - offset) + ((double) tv.tv_usec) / 1000000.0;
}

void Time::Convert(struct timeval& tv) const {
    if (! initialized) Initialize();

    tv.tv_sec  = (long) floor(rep + offset);
    tv.tv_usec = (long) round((rep + offset - tv.tv_sec) * 1000000.0);
}

#if 0
time_t timezone_to_local(time_t clock, const char *tz) {
    const char *old=getenv("TZ");
    if (old) old=strdupa(old);
    
    struct tm* t = localtime(&clock);
    setenv("TZ", tz, 1);
    tzset();
    
    clock = mktime(t);
    if (old) setenv("TZ", old, 1); else unsetenv("TZ");
    tzset();

    return clock;
}

time_t local_to_timezone(time_t clock, const char *tz) {
    const char *old=getenv("TZ");
    if (old) old=strdupa(old);
    
    setenv("TZ", tz, 1);
    tzset();
    struct tm* t = localtime(&clock);
    
    if (old) setenv("TZ", old, 1); else unsetenv("TZ");
    tzset();
    clock = mktime(t);

    return clock;
}
#endif

Duration::Duration(const struct timeval& tv) {
    rep = tv.tv_sec + ((double) tv.tv_usec) / 1000000.0;
}

void Duration::Convert(struct timeval& tv) const {
    tv.tv_sec  = (long) floor(rep);
    tv.tv_usec = (long) round((rep - tv.tv_sec) / 1000000.0);
}
