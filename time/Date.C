/* Copyright (c) 1993 by Sanjay Ghemawat */

#include "Date.h"

/*
 * Epoch is Thursday January 1, 1970.
 */ 
int Date::epochRep = 25567; // 70*365.25
int Date::epochWDayIndex = 5;

Date::Date(Time t) {
    int mday, year;
    Month month;
    WeekDay wday;

    t.BreakDownDate(mday, wday, month, year);
    rep = GetRep(mday, month, year);
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
    return WeekDay::First() + epochWDayIndex - 1 + rep - epochRep;
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

