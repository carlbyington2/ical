/* Copyright (c) 1993 by Sanjay Ghemawat */

#include "Month.h"
#include "Year.h"

char const* Month::name[13] = {
    0,
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
    };

char const* Month::shortName[13] = {
    0,
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
    };

int Month::normalSize[13] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
int Month::leapSize[13]   = { 0,31,29,31,30,31,30,31,31,30,31,30,31 };

int Month::normalOffset[13] = {
    0,
    0,          /* Jan */
    31,         /* Feb */
    59,         /* Mar */
    90,         /* Apr */
    120,        /* May */
    151,        /* Jun */
    181,        /* Jul */
    212,        /* Aug */
    243,        /* Sep */
    273,        /* Oct */
    304,        /* Nov */
    334         /* Dec */
    };

int Month::leapOffset[13] = {
    0,
    0,          /* Jan */
    31,         /* Feb */
    60,         /* Mar */
    91,         /* Apr */
    121,        /* May */
    152,        /* Jun */
    182,        /* Jul */
    213,        /* Aug */
    244,        /* Sep */
    274,        /* Oct */
    305,        /* Nov */
    335         /* Dec */
    };

int Month::Size(int year) const {
    return (Year::IsLeap(year) ? LeapSize() : NormalSize());
}

int Month::Offset(int year) const {
    return (Year::IsLeap(year) ? LeapOffset() : NormalOffset());
}

void Month::Normalize() {
    int val = rep - 1;

    if (val < 0) {
        val = 12 - (0 - val) % 12;
        if (val == 12) val = 0;
    }
    else {
        val = val % 12;
    }

    rep = val + 1;
}
