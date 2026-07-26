/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef TIMEH
#define TIMEH

#include <time.h>
#include <sys/time.h>

class Time;
class Duration;
class WeekDay;
class Month;

/*
 * A class to represent system time.
 */
class Time {
  public:
    /*
     * Constructors and assignments.
     */
    Time();                     /* Unspecified time */
    Time(Time const&);          /* Copy other time */
    Time(double seconds);       /* Seconds since unspecified epoch */

    Time& operator=(Time const&);
    static Time Now();

    /* Addition and subtraction */
    inline friend Time          operator+ (Time const&, Duration const&);
    inline friend Time          operator- (Time const&, Duration const&);
    inline friend Duration      operator- (Time const&, Time const&);
    Time&                       operator+=(Duration const&);
    Time&                       operator-=(Duration const&);

    /* Comparisons */
    inline friend int operator == (Time const&, Time const&);
    inline friend int operator != (Time const&, Time const&);
    inline friend int operator <  (Time const&, Time const&);
    inline friend int operator <= (Time const&, Time const&);
    inline friend int operator >  (Time const&, Time const&);
    inline friend int operator >= (Time const&, Time const&);

    /*
     * Various conversions.
     */

    /* Variables for default references */
    static int junkInt;

    void BreakDownDate(int&     mday,
                       WeekDay& wday,
                       Month&   month,
                       int&     year) const;

    void BreakDownClock(int& hour     = junkInt,
                        int& minute   = junkInt,
                        int& second   = junkInt,
                        int& millisec = junkInt) const;

    void BreakDown(int&     mday,
                   WeekDay& wday,
                   Month&   month,
                   int&     year,
                   int&     hour     = junkInt,
                   int&     minute   = junkInt,
                   int&     second   = junkInt,
                   int&     millisec = junkInt,
                   const char *tz=0) const;

    /* UN*X specific */
    /* Copy UN*X time structure */
    Time(struct timeval const&);
    void Convert(struct timeval&) const;

    double EpochSeconds() const;        /* Return seconds since epoch */
  private:
    /*
     * Rep is the number of seconds since some epoch.
     */
    double rep;

    /* Time initialization stuff. */
    static int    initialized;
    static double offset;

    static void Initialize();
};

/*
 * A class to represent a time interval.
 */
class Duration {
  public:
    /* Constructors and assignments */
    Duration();
    Duration(Duration const&);
    Duration& operator=(Duration const&);

    /* Special durations */
    static Duration Week();
    static Duration Day();
    static Duration Hour();
    static Duration Minute();
    static Duration Second();
    static Duration MilliSecond();
    static Duration MicroSecond();

    /* Addition and subtraction */
    inline friend Time          operator+ (Time const&, Duration const&);
    inline friend Time          operator- (Time const&, Duration const&);
    inline friend Duration      operator- (Time const&, Time const&);

    inline friend Duration      operator+ (Duration const&, Duration const&);
    inline friend Duration      operator- (Duration const&, Duration const&);

    /* Multiplication and division */
    inline friend Duration      operator* (Duration const&, int);
    inline friend Duration      operator* (Duration const&, double);
    inline friend Duration      operator/ (Duration const&, int);
    inline friend Duration      operator/ (Duration const&, double);

    Duration&   operator += (Duration const&);
    Duration&   operator -= (Duration const&);
    Duration&   operator *= (int);
    Duration&   operator *= (double);
    Duration&   operator /= (int);
    Duration&   operator /= (double);

    /* Comparisons (ints represent seconds) */
    inline friend int operator == (Duration const&, Duration const&);
    inline friend int operator != (Duration const&, Duration const&);
    inline friend int operator <  (Duration const&, Duration const&);
    inline friend int operator <= (Duration const&, Duration const&);
    inline friend int operator >  (Duration const&, Duration const&);
    inline friend int operator >= (Duration const&, Duration const&);

    /* Conversion */
    double Weeks() const;
    double Days() const;
    double Hours() const;
    double Minutes() const;
    double Seconds() const;
    double MilliSeconds() const;
    double MicroSeconds() const;

    /* UN*X Specific */
    Duration(struct timeval const&);
    void Convert(struct timeval&) const;
  private:
    /*
     * REP is the number of seconds in the Duration.
     */
    double rep;

    Duration(double);
};

/*
 * Time.
 */
inline Time::Time() { }

inline Time::Time(const Time& src) {
    rep = src.rep;
}

inline Time& Time::operator=(Time const& src) {
    rep = src.rep;
    return *this;
}

inline Time::Time(double r) {
    rep = r;
}

inline int operator == (Time const& a, Time const& b) {
    return a.rep == b.rep;
}

inline int operator != (Time const& a, Time const& b) {
    return a.rep != b.rep;
}

inline int operator <  (Time const& a, Time const& b) {
    return a.rep < b.rep;
}

inline int operator <= (Time const& a, Time const& b) {
    return a.rep <= b.rep;
}

inline int operator >  (Time const& a, Time const& b) {
    return a.rep > b.rep;
}

inline int operator >= (Time const& a, Time const& b) {
    return a.rep >= b.rep;
}

inline double Time::EpochSeconds() const {
    return rep;
}

/*
 * Duration.
 */
inline Duration::Duration() { }

inline Duration::Duration(Duration const& src) {
    rep = src.rep;
}

inline Duration& Duration::operator=(const Duration& src) {
    rep = src.rep;
    return *this;
}

inline Duration::Duration(double r) {
    rep = r;
}

inline Duration Duration::Week() {
    Duration result(7.0 * 24.0 * 60.0 * 60.0);
    return result;
}

inline Duration Duration::Day() {
    Duration result(24.0 * 60.0 * 60.0);
    return result;
}

inline Duration Duration::Hour() {
    Duration result(60.0 * 60.0);
    return result;
}

inline Duration Duration::Minute() {
    Duration result(60.0);
    return result;
}

inline Duration Duration::Second() {
    Duration result(1.0);
    return result;
}

inline Duration Duration::MilliSecond() {
    Duration result(1e-3);
    return result;
}

inline Duration Duration::MicroSecond() {
    Duration result(1e-6);
    return result;
}

inline int operator == (Duration const& a, Duration const& b) {
    return a.rep == b.rep;
}

inline int operator != (Duration const& a, Duration const& b) {
    return a.rep != b.rep;
}

inline int operator <  (Duration const& a, Duration const& b) {
    return a.rep < b.rep;
}

inline int operator <= (Duration const& a, Duration const& b) {
    return a.rep <= b.rep;
}

inline int operator >  (Duration const& a, Duration const& b) {
    return a.rep > b.rep;
}

inline int operator >= (Duration const& a, Duration const& b) {
    return a.rep >= b.rep;
}

inline double Duration::Weeks() const {
    return (rep / (7.0 * 24.0 * 60.0 * 60.0));
}

inline double Duration::Days() const {
    return (rep / (24.0 * 60.0 * 60.0));
}

inline double Duration::Hours() const {
    return (rep / (60.0 * 60.0));
}

inline double Duration::Minutes() const {
    return (rep / (60.0));
}

inline double Duration::Seconds() const {
    return (rep);
}

inline double Duration::MilliSeconds() const {
    return (rep * 1000.0);
}

inline double Duration::MicroSeconds() const {
    return (rep * 1000000.0);
}

/*
 * Arithmetic
 */
inline Time operator + (Time const& t, Duration const& d) {
    return Time(t.rep + d.rep);
}

inline Time operator - (Time const& t, Duration const& d) {
    return Time(t.rep - d.rep);
}

inline Duration operator - (Time const& a, Time const& b) {
    return Duration(a.rep - b.rep);
}

inline Duration operator + (Duration const& a, Duration const& b) {
    return Duration(a.rep + b.rep);
}

inline Duration operator - (Duration const& a, Duration const& b) {
    return Duration(a.rep - b.rep);
}

inline Time& Time::operator += (Duration const& d) {
    rep += d.Seconds();
    return *this;
}

inline Time& Time::operator -= (Duration const& d) {
    rep -= d.Seconds();
    return *this;
}

inline Duration operator * (Duration const& d, int x) {
    return Duration(d.rep * double(x));
}

inline Duration operator * (Duration const& d, double x) {
    return Duration(d.rep * x);
}

inline Duration operator / (Duration const& d, int x) {
    return Duration(d.rep / double(x));
}

inline Duration operator / (Duration const& d, double x) {
    return Duration(d.rep / x);
}

inline Duration& Duration::operator += (Duration const& d) {
    rep += d.rep;
    return *this;
}

inline Duration& Duration::operator -= (Duration const& d) {
    rep -= d.rep;
    return *this;
}

inline Duration& Duration::operator *= (int x) {
    rep *= double(x);
    return *this;
}

inline Duration& Duration::operator *= (double x) {
    rep *= x;
    return *this;
}

inline Duration& Duration::operator /= (int x) {
    rep /= double(x);
    return *this;
}

inline Duration& Duration::operator /= (double x) {
    rep /= x;
    return *this;
}

#endif /*TIMEH */
