/* Copyright (c) 1993 by Sanjay Ghemawat */

#include "Date.h"
#include "Time.h"
#include "Month.h"
#include "WeekDay.h"
#include "Year.h"

int Date::epochRep = 0;
int Date::epochWDayIndex = 0;
int Date::epochInitialized = 0;

int Date::GetRep(int d, Month m, int y) {
    return Year::Offset(y) + m.Offset(y) + d - 1;
}

Date::Date(int d, Month m, int y) {
    rep = GetRep(d, m, y);
}

Date::Date(Time t) {
    int mday, year;
    Month month;
    WeekDay wday;

    t.BreakDownDate(mday, wday, month, year);
    rep = GetRep(mday, month, year);
}

Date Date::Today() {
    return Date(Time::Now());
}

Date Date::First() {
    return Date(1, Month::First(), Year::First());
}

Date Date::Last() {
    Month lastMonth = Month::Last();
    int   lastYear  = Year::Last();

    return Date(lastMonth.Size(lastYear), lastMonth, lastYear);
}

void Date::BreakDown(int& d, WeekDay& wd, Month& m, int& y) const {
    long days = rep;

    /* Find year */
    int year = Year::First();
    while (1) {
	int size = Year::Size(year);

	if (days < size) {
	    /* Date occurs in year */
	    break;
	}
	days -= size;
	year++;
    }
    y = year;

    /* Find month within year */
    int leap = Year::IsLeap(y);
    Month month = Month::First();
    while (month != Month::Last()) {
	int msize = (leap ? month.LeapSize() : month.NormalSize());
	if (days < msize) {
	    /* Found month */
	    break;
	}
	days -= msize;
	month += 1;
    }
    m = month;

    d = days + 1;
    wd = GetWDay();
}

int Date::GetMDay() const {
    int d;
    WeekDay wd;
    Month m;
    int y;

    BreakDown(d, wd, m, y);
    return d;
}

WeekDay Date::GetWDay() const {
    if (! epochInitialized) {
	InitializeEpoch();
    }

    return (WeekDay::First() + epochWDayIndex - 1) + (rep - epochRep);
}

Month Date::GetMonth() const {
    int d;
    WeekDay wd;
    Month m;
    int y;

    BreakDown(d, wd, m, y);
    return m;
}

int Date::GetYear() const {
    int d;
    WeekDay wd;
    Month m;
    int y;

    BreakDown(d, wd, m, y);
    return y;
}

/*
 * Epoch is Thursday January 1, 1970.
 */ 
void Date::InitializeEpoch() {
    epochInitialized = 1;
    epochRep = GetRep(1, Month::January(), 1970);
    epochWDayIndex = WeekDay::Thursday().Index();
}
