/* Copyright (c) 1993 by Sanjay Ghemawat */

#include <stddef.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include "config.h"

#include "Month.h"
#include "WeekDay.h"
#include "Time.h"

#ifndef HAVE_GETTIMEOFDAY_PROTO
extern "C" {
    extern int gettimeofday(struct timeval*, struct timezone*);
}
#endif

static double roundit(double d) {
    double x = floor(d);
    if ((d - x) >= 0.5) {
	x += 1.0;
    }
    return x;
}

int Time::junkInt = 0;

/*
 * Want to get high resolution for region of time that will be most
 * heavily used.  This region will probably be around now. Therefore,
 * rep will be number of seconds elapsed since we construct the first
 * Time value for this run of the program.
 */

int    Time::initialized = 0;	/* Initialized yet? */
double Time::offset = 0.0;	/* Offset for time values */

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
		     int& hour, int& min, int& sec, int& milli) const
{
    if (! initialized) Initialize();

    time_t clock = (time_t) floor(rep + offset);
    milli = (int) roundit((rep + offset - clock) * 1000.0);
    while (milli >= 1000) {
	milli -= 1000;
	clock++;
    }

    struct tm* t = localtime(&clock);

    mday  = t->tm_mday;				/* tm_mday in 1..31 */
    wday  = WeekDay::Sunday() + t->tm_wday;	/* tm_wday in 0..6.  Sun = 0 */
    month = Month::January() + t->tm_mon;	/* tm_mon  in 0..11. Jan = 0 */
    year  = t->tm_year + 1900;
    hour  = t->tm_hour;
    min   = t->tm_min;
    sec   = t->tm_sec;
}

Time::Time(const struct timeval& tv) {
    if (! initialized) Initialize();
    rep = (tv.tv_sec - offset) + ((double) tv.tv_usec) / 1000000.0;
}

void Time::Convert(struct timeval& tv) const {
    if (! initialized) Initialize();

    tv.tv_sec  = (long) floor(rep + offset);
    tv.tv_usec = (long) roundit((rep + offset - tv.tv_sec) * 1000000.0);
}

Duration::Duration(const struct timeval& tv) {
    rep = tv.tv_sec + ((double) tv.tv_usec) / 1000000.0;
}

void Duration::Convert(struct timeval& tv) const {
    tv.tv_sec  = (long) floor(rep);
    tv.tv_usec = (long) roundit((rep - tv.tv_sec) / 1000000.0);
}
