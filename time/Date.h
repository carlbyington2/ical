/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _DATEH
#define _DATEH

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
    Date();
    Date(Date const&);
    Date(int day, Month month, int year);
    Date(Time);
    Date(long days);	/* Days since unspecified epoch */

    Date& operator=(Date const&);

    static Date Today();
    static Date First();
    static Date Last();

    /*
     * Addition and subtraction - number of days represents interval
     * between two dates.
     */
    inline friend Date	operator + (Date, int days);
    inline friend Date	operator - (Date, int days);
    inline friend int	operator - (Date, Date);

    Date&		operator += (int);
    Date&		operator -= (int);

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
    int		GetMDay()  const;	/* Day of the month. */
    WeekDay	GetWDay()  const;	/* Day of the week. */
    Month   	GetMonth() const;
    int		GetYear()  const;

    void BreakDown(int&, WeekDay&, Month&, int&) const;

    /*
     * This needs to be implemented.
     *
     * Time Midnight() const;
     */

    long EpochDays() const;	/* Return days since epoch */
  private:
    /*
     * Rep is number of days since some epoch.
     */
    long rep;

    static int GetRep(int, Month, int);

    /*
     * Cached rep and weekday index for some epoch date.
     */
    static int epochRep;
    static int epochWDayIndex;
    static int epochInitialized;

    static void InitializeEpoch();
};

inline Date::Date() {
    rep = 0;
}

inline Date::Date(long r) {
    rep = r;
}

inline Date::Date(Date const& d) {
    rep = d.rep;
}

inline Date& Date::operator = (Date const& d) {
    rep = d.rep;
    return *this;
}

inline Date operator + (Date d, int days) {
    return Date(d.rep + days);
}

inline Date operator - (Date d, int days) {
    return Date(d.rep - days);
}

inline int operator - (Date a, Date b) {
    return (a.rep - b.rep);
}

inline Date& Date::operator += (int days) {
    rep += days;
    return *this;
}

inline Date& Date::operator -= (int days) {
    rep -= days;
    return *this;
}

inline int operator == (Date a, Date b) {
    return (a.rep == b.rep);
}

inline int operator != (Date a, Date b) {
    return (a.rep != b.rep);
}

inline int operator < (Date a, Date b) {
    return (a.rep < b.rep);
}

inline int operator <= (Date a, Date b) {
    return (a.rep <= b.rep);
}

inline int operator > (Date a, Date b) {
    return (a.rep > b.rep);
}

inline int operator >= (Date a, Date b) {
    return (a.rep >= b.rep);
}

inline long Date::EpochDays() const {
    return rep;
}

#endif /* _DATEH */
