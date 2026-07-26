/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _YEARH
#define _YEARH

class Year {
  public:
    static int IsLeap(int);
    static int Size(int);

    /*
     * Limit range of years.
     */
    static int First();
    static int Last();

    /*
     * Find offset of given year from beginning of first year.
     * Offset of First() year is zero. Offset of First() + 1
     * year is equal to the size of the first year.
     */
    static long Offset(int year);
  private:
    static int           FirstCentury();
    static long          Century_Offset(int);
    static unsigned long Century_Size(int);

    static int  first;
    static int  last;
    static int  first_century;
    static int  first_year_off; /* Diff between first, first_century */
    static long first_day_off;
};

inline int Year::IsLeap(int y) {
    return (((y % 4) == 0) &&
            (((y % 100) != 0) || ((y % 400) == 0))
            );
}

inline int Year::Size(int y) {
    return IsLeap(y) ? 366 : 365;
}

inline int Year::First() {
    return first;
}

inline int Year::Last() {
    return last;
}

inline int Year::FirstCentury() {
    return first_century;
}

#endif /* _YEARH */
