/* Copyright (c) 1993 by Sanjay Ghemawat */

#include <stddef.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>
#ifndef __FreeBSD__
#include <alloca.h>
#endif /* __FreeBSD__ */
#include <stdlib.h>
#include <stdio.h>

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

char *Time::toISO8601() const {
    if (! initialized) Initialize();
    // yyyymmddThhmmssZ
    static char t[20];
    int     DD;
    WeekDay wday;
    Month   month;
    int     YYYY;
    int     hh, mm, ss;
    int     milli;
    BreakDown(DD, wday, month, YYYY, hh, mm, ss, milli, "UTC");
    int MM = month.Index();
    snprintf(t, sizeof(t), "%04d%02d%02dT%02d%02d%02dZ", YYYY, MM, DD, hh, mm, ss);
    return t;
}

bool Time::fromISO8601(char const* isodatetime) {
    if (! initialized) Initialize();
    // yyyymmddThhmmssZ
    int YYYY,MM,DD,hh,mm,ss;
    int rc = sscanf(isodatetime, "%04d%02d%02dT%02d%02d%02dZ", &YYYY, &MM, &DD, &hh, &mm, &ss);
    if (rc != 6) return false;
    struct tm utctime = {0};
    utctime.tm_sec   = ss;
    utctime.tm_min   = mm;
    utctime.tm_hour  = hh;
    utctime.tm_mday  = DD;
    utctime.tm_mon   = MM - 1;
    utctime.tm_year  = YYYY - 1900;
    utctime.tm_wday  = 0;
    utctime.tm_yday  = 0;
    utctime.tm_isdst = 0;
    time_t tt = timegm(&utctime);
    if (tt < 0) return false;
    rep = tt - offset;
    return true;
}

Duration::Duration(const struct timeval& tv) {
    rep = tv.tv_sec + ((double) tv.tv_usec) / 1000000.0;
}

void Duration::Convert(struct timeval& tv) const {
    tv.tv_sec  = (long) floor(rep);
    tv.tv_usec = (long) round((rep - tv.tv_sec) / 1000000.0);
}
