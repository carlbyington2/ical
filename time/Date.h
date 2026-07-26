/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _DATEH
#define _DATEH

#include "Month.h"
#include "WeekDay.h"
#include "Year.h"
#include "WeekDay.h"
#include "Time_.h"

class Time;
class Month;
class WeekDay;

/*
 * Date class.
 */
class Date {
  public:
    /*
     * Constructors and assignments.
     */
    Date() : rep(0) {}
    Date(Date const& d) : rep(d.rep){}
    Date(int day, Month month, int year) { rep = GetRep(day, month, year); }
    Date(Time);
    Date(long days) : rep(days) {}

    Date& operator=(Date const& d) { rep = d.rep; return *this; }
    Date& operator += (int days) { rep += days; return *this; }
    Date& operator -= (int days) { rep -= days; return *this; }

    static Date Today() { return Date(Time::Now()); }
    static Date First() { return Date(1, Month::First(), Year::First()); }
    static Date Last();

    /*
     * Addition and subtraction - number of days represents interval
     * between two dates.
     */
    inline friend Date  operator + (Date, int days);
    inline friend Date  operator - (Date, int days);
    inline friend int   operator - (Date, Date);


    /*
     * Comparisons.
     */
    inline friend int operator == (Date, Date);
    inline friend int operator != (Date, Date);
    inline friend int operator <  (Date, Date);
    inline friend int operator <= (Date, Date);
    inline friend int operator >  (Date, Date);
    inline friend int operator >= (Date, Date);

    /*
     * Conversions.
     */
    int         GetMDay()  const;       /* Day of the month. */
    WeekDay     GetWDay()  const;       /* Day of the week. */
    Month       GetMonth() const;
    int         GetYear()  const;

    void BreakDown(int&, WeekDay&, Month&, int&) const;

    /*
     * This needs to be implemented.
     *
     * Time Midnight() const;
     */

    int EpochDays() const { return rep; }
    static int epochRep;
  private:
    static int epochWDayIndex;
    /*
     * Rep is number of days since some epoch.
     */
    long rep;

    static int GetRep(int d, Month m, int y)
    { return Year::Offset(y) + m.Offset(y) + d - 1; }
};

inline Date operator +  (Date d, int days) { return Date(d.rep + days); }
inline Date operator -  (Date d, int days) { return Date(d.rep - days); }
inline int  operator -  (Date a, Date b) { return (a.rep -  b.rep); }
inline int  operator == (Date a, Date b) { return (a.rep == b.rep); }
inline int  operator != (Date a, Date b) { return (a.rep != b.rep); }
inline int  operator <  (Date a, Date b) { return (a.rep <  b.rep); }
inline int  operator <= (Date a, Date b) { return (a.rep <= b.rep); }
inline int  operator >  (Date a, Date b) { return (a.rep >  b.rep); }
inline int  operator >= (Date a, Date b) { return (a.rep >= b.rep); }

#endif /* _DATEH */
