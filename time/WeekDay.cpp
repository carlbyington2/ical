/* Copyright (c) 1993 by Sanjay Ghemawat */

#include "WeekDay.h"

char const* WeekDay::name[8] = {
    0,
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
    };

char const* WeekDay::shortName[8] = {
    0,
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
    };

void WeekDay::Normalize() {
    int val = rep - 1;

    if (val < 0) {
        val = 7 - (0 - val) % 7;
        if (val == 7) val = 0;
    }
    else {
        val = val % 7;
    }

    rep = val + 1;
}
