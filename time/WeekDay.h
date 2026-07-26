/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _WEEKDAYH
#define _WEEKDAYH

/*
 * Sunday is first, Saturday is last.
 */

class WeekDay {
  public:
    /* Constructors */
    WeekDay();
    WeekDay(WeekDay const&);
    WeekDay& operator = (WeekDay);

    static WeekDay Sunday();
    static WeekDay Monday();
    static WeekDay Tuesday();
    static WeekDay Wednesday();
    static WeekDay Thursday();
    static WeekDay Friday();
    static WeekDay Saturday();
    static WeekDay First();
    static WeekDay Last();

    /* Observers */
    char const* Name() const;
    char const* ShortName() const;
    int Index() const;

    /* Arithemtic - wraps around */
    WeekDay& operator += (int);
    WeekDay& operator -= (int);

    /* Comparison */
    inline friend int operator == (WeekDay, WeekDay);
    inline friend int operator != (WeekDay, WeekDay);

    /* Binary arithmetic */
    inline friend WeekDay operator + (WeekDay, int);
    inline friend WeekDay operator - (WeekDay, int);
    inline friend int     operator - (WeekDay, WeekDay);
  private:
    int rep;

    WeekDay(int number);
    int Number() const;
    void Normalize();

    static char const* name[8];
    static char const* shortName[8];
};

inline WeekDay::WeekDay() { }

inline WeekDay::WeekDay(int number) {
    rep = number;
    Normalize();
}

inline WeekDay::WeekDay(WeekDay const& x) {
    rep = x.rep;
}

inline WeekDay& WeekDay::operator = (WeekDay x) {
    rep = x.rep;
    return *this;
}

inline WeekDay WeekDay::Sunday() {
    return WeekDay(1);
}

inline WeekDay WeekDay::Monday() {
    return WeekDay(2);
}

inline WeekDay WeekDay::Tuesday() {
    return WeekDay(3);
}

inline WeekDay WeekDay::Wednesday() {
    return WeekDay(4);
}

inline WeekDay WeekDay::Thursday() {
    return WeekDay(5);
}

inline WeekDay WeekDay::Friday() {
    return WeekDay(6);
}

inline WeekDay WeekDay::Saturday() {
    return WeekDay(7);
}

inline WeekDay WeekDay::First() {
    return WeekDay(1);
}

inline WeekDay WeekDay::Last() {
    return WeekDay(7);
}

inline int WeekDay::Number() const {
    return rep;
}

inline int WeekDay::Index() const {
    return rep;
}

inline char const* WeekDay::Name() const {
    return name[rep];
}

inline char const* WeekDay::ShortName() const {
    return shortName[rep];
}

inline WeekDay& WeekDay::operator += (int i) {
    rep += i;
    Normalize();
    return *this;
}

inline WeekDay& WeekDay::operator -= (int i) {
    rep -= i;
    Normalize();
    return *this;
}

inline int operator == (WeekDay a, WeekDay b) {
    return (a.Number() == b.Number());
}

inline int operator != (WeekDay a, WeekDay b) {
    return (a.Number() != b.Number());
}

inline WeekDay operator + (WeekDay a, int i) {
    return WeekDay(a.Number() + i);
}

inline WeekDay operator - (WeekDay a, int i) {
    return WeekDay(a.Number() - i);
}

inline int operator - (WeekDay a, WeekDay b) {
    return (a.Number() - b.Number());
}

#endif /* _WEEKDAYH */
