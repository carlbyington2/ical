/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef SMALLINTSETH
#define SMALLINTSETH

class Lexer;
class charArray;

static const int SISetLargestMember = 31;

/*
 * A Set of small integers (range 0..SISetLargestMember)
 */
class SmallIntSet {
  public:
    SmallIntSet();
    SmallIntSet(const SmallIntSet&);
    ~SmallIntSet();

    SmallIntSet& operator= (const SmallIntSet&);

    /*
     * Set operations.
     */
    void Clear();
    void Insert(unsigned int);
    void Remove(unsigned int);
    int Member(unsigned int) const;

    /*
     * Comparison.
     */
    int operator == (SmallIntSet);
    int operator != (SmallIntSet);

    /*
     * Set union.
     */
    inline friend SmallIntSet operator+(const SmallIntSet, const SmallIntSet);

    /*
     * Set intersection.
     */
    inline friend SmallIntSet operator*(const SmallIntSet, const SmallIntSet);

    /*
     * Set subtraction.
     */
    inline friend SmallIntSet operator-(const SmallIntSet, const SmallIntSet);

    /*
     * Set size.
     */
    unsigned int Size() const;

    /*
     * I/O.
     */
    int Read(Lexer*);
    void Write(charArray*) const;
  protected:
    unsigned long bits;

    SmallIntSet(unsigned long rep);
};

inline SmallIntSet::SmallIntSet()                       { bits = 0; }
inline SmallIntSet::SmallIntSet(const SmallIntSet& s)   { bits = s.bits; }
inline SmallIntSet::SmallIntSet(unsigned long rep)      { bits = rep; }
inline SmallIntSet::~SmallIntSet()                      { }
inline SmallIntSet& SmallIntSet::operator=(const SmallIntSet& s) {
    bits = s.bits;
    return *this;
}

inline void SmallIntSet::Clear() {
    bits = 0;
}

inline void SmallIntSet::Insert(unsigned int x) {
    if (x <= SISetLargestMember)
        bits |= (1 << x);
}

inline void SmallIntSet::Remove(unsigned int x) {
    bits &= ~(1 << x);
}

inline int SmallIntSet::Member(unsigned int x) const {
    return (bits & (1 << x));
}

inline int SmallIntSet::operator == (SmallIntSet s) {
    return bits == s.bits;
}

inline int SmallIntSet::operator != (SmallIntSet s) {
    return bits != s.bits;
}

inline SmallIntSet operator+(const SmallIntSet a, const SmallIntSet b) {
    return SmallIntSet(a.bits | b.bits);
}

inline SmallIntSet operator*(const SmallIntSet a, const SmallIntSet b) {
    return SmallIntSet(a.bits & b.bits);
}

inline SmallIntSet operator-(const SmallIntSet a, const SmallIntSet b) {
    return SmallIntSet(a.bits & (a.bits ^ b.bits));
}

#endif /*SMALLINTSETH */
