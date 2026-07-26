/* Copyright (c) 1996  by Sanjay Ghemawat */
#include <stdio.h>
#include <string.h>
#include "Date.h"
#include "Month.h"
#include "WeekDay.h"
#include "Year.h"
#include "parse.h"

static void debug_date(char const* str) {
    Date result;
    int start, length;

    while (find_date(str, result, start, length)) {
        char* spec = new char[length+1];
        strncpy(spec, str+start, length);
        spec[length] = '\0';

        int day, year;
        WeekDay wday;
        Month month;
        result.BreakDown(day, wday, month, year);

        printf("%-30s = [%s %s %d, %d]\n",
               spec, wday.Name(), month.Name(), day, year);

        delete [] spec;

        str += start + length;
    }
}

static void print_tod(int t) {
    int h, m, s;
    s = t % 60;
    t = t / 60;
    m = t % 60;
    t = t / 60;
    h = t;

    char const* ampm = (h >= 12) ? "pm" : "am";
    h = h % 12;
    if (h == 0) h = 12;

    printf("%02d:%02d:%02d%s", h, m, s, ampm);
}

static void debug_time(char const* str) {
    int result;
    int start, length;

    while (find_timeofday(str, result, start, length)) {
        char* spec = new char[length+1];
        strncpy(spec, str+start, length);
        spec[length] = '\0';

        printf("%-30s = [", spec);
        print_tod(result);
        printf("]\n");

        delete [] spec;

        str += start + length;
    }
}

static void debug_range(char const* str) {
    int start, length;
    int tstart, tfinish;

    while (find_timerange(str, tstart, tfinish, start, length)) {
        char* spec = new char[length+1];
        strncpy(spec, str+start, length);
        spec[length] = '\0';

        printf("%-30s = [", spec);
        print_tod(tstart);
        printf(" -- ");
        print_tod(tfinish);
        printf("]\n");

        delete [] spec;

        str += start + length;
    }
}

int
main() {
    char line[1000];

    while (gets(line) != 0) {
        if (strncmp(line, "date", 4) == 0)
            debug_date(line);
        else if (strncmp(line, "time", 4) == 0)
            debug_time(line);
        else if (strncmp(line, "range", 5) == 0)
            debug_range(line);
        else {
            debug_date(line);
            debug_time(line);
            debug_range(line);
        }
    }

    return 0;
}
