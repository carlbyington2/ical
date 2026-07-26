/* Copyright (c) 1993 by Sanjay Ghemawat */

#include "Year.h"

#define FIRST_YEAR      1900
#define LAST_YEAR       2400
#define FIRST_CENTURY   ((FIRST_YEAR - 1) / 100)
#define FIRST_YEAR_OFF  (FIRST_YEAR - 100*FIRST_CENTURY)

int Year::first                 = FIRST_YEAR;
int Year::last                  = LAST_YEAR;
int Year::first_century         = FIRST_CENTURY;
int Year::first_year_off        = FIRST_YEAR_OFF;
long Year::first_day_off        = (long((FIRST_YEAR_OFF - 1) / 4) +
                                   365L * long(FIRST_YEAR_OFF - 1));

unsigned long Year::Century_Size(int c) {
    return (((c + 1) % 4) == 0) ? 36525L : 36524L;
}

long Year::Century_Offset(int c) {
    long offset = 0L;
    int i;

    for (i = Year::FirstCentury(); i < c; i++) {
        offset += Century_Size(i);
    }

    for (i = Year::FirstCentury() - 1; i >= c; i--) {
        offset -= Century_Size(i);
    }

    return offset - first_day_off;
}

long Year::Offset(int year) {
    int c = (year - 1) / 100;
    int z = year - 100 * c;

    return Century_Offset(c) + ((z - 1) / 4) + 365 * (z - 1);
}
