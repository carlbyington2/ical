/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _MONTHH
#define _MONTHH

class Month {
  public:
    /* Constructors */
    Month();
    Month(Month const&);
    Month& operator = (Month);

    static Month January();
    static Month February();
    static Month March();
    static Month April();
    static Month May();
    static Month June();
    static Month July();
    static Month August();
    static Month September();
    static Month October();
    static Month November();
    static Month December();
    static Month First();
    static Month Last();

    /* Observers */
    char const* Name() const;
    char const* ShortName() const;
    int         Index() const;

    int         NormalSize() const;     /* Size in non-leap years */
    int         LeapSize() const;       /* Size in leap years */
    int         Size(int y) const;      /* Size in given year */

    int         NormalOffset() const;   /* Offset within non-leap year */
    int         LeapOffset() const;     /* Offset within leap year */
    int         Offset(int y) const;    /* Offset within given year */

    /* Arithemtic - wraps around. */
    Month& operator += (int);
    Month& operator -= (int);

    /* Comparison */
    inline friend int operator == (Month, Month);
    inline friend int operator != (Month, Month);

    /* Binary arithmetic */
    inline friend Month operator + (Month, int);
    inline friend Month operator - (Month, int);
    inline friend int   operator - (Month, Month);
  private:
    int rep;

    static char const* name[13];
    static char const* shortName[13];
    static int normalSize[13];
    static int leapSize[13];
    static int normalOffset[13];
    static int leapOffset[13];

    Month(int number);
    int Number() const;
    void Normalize();
};

inline Month::Month() { }

inline Month::Month(int number) {
    rep = number;
    Normalize();
}

inline Month::Month(Month const& x) {
    rep = x.rep;
}

inline Month& Month::operator = (Month x) {
    rep = x.rep;
    return *this;
}

inline Month Month::January() {
    return Month(1);
}

inline Month Month::February() {
    return Month(2);
}

inline Month Month::March() {
    return Month(3);
}

inline Month Month::April() {
    return Month(4);
}

inline Month Month::May() {
    return Month(5);
}

inline Month Month::June() {
    return Month(6);
}

inline Month Month::July() {
    return Month(7);
}

inline Month Month::August() {
    return Month(8);
}

inline Month Month::September() {
    return Month(9);
}

inline Month Month::October() {
    return Month(10);
}

inline Month Month::November() {
    return Month(11);
}

inline Month Month::December() {
    return Month(12);
}

inline Month Month::First() {
    return Month(1);
}

inline Month Month::Last() {
    return Month(12);
}

inline int Month::Number() const {
    return rep;
}

inline int Month::Index() const {
    return rep;
}

inline char const* Month::Name() const {
    return name[rep];
}

inline char const* Month::ShortName() const {
    return shortName[rep];
}

inline int Month::NormalSize() const {
    return normalSize[rep];
}

inline int Month::LeapSize() const {
    return leapSize[rep];
}

inline int Month::NormalOffset() const {
    return normalOffset[rep];
}

inline int Month::LeapOffset() const {
    return leapOffset[rep];
}

inline Month& Month::operator += (int i) {
    rep += i;
    Normalize();
    return *this;
}

inline Month& Month::operator -= (int i) {
    rep -= i;
    Normalize();
    return *this;
}

inline int operator == (Month a, Month b) {
    return (a.Number() == b.Number());
}

inline int operator != (Month a, Month b) {
    return (a.Number() != b.Number());
}

inline Month operator + (Month a, int i) {
    return Month(a.Number() + i);
}

inline Month operator - (Month a, int i) {
    return Month(a.Number() - i);
}

inline int operator - (Month a, Month b) {
    return (a.Number() - b.Number());
}

#endif /* _MONTHH */
