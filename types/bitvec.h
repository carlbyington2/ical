/* Copyright (c) 1995 by Sanjay Ghemawat */
#ifndef _BITVEC_H
#define _BITVEC_H

// A "BitVec" is an array of bits.  The size of the array is fixed at
// creation time.

#include <assert.h>

typedef unsigned long int bitvword;

class BitVec {
  public:
    BitVec(int length);
    // requires "length >= 0".
    // effects  Create bit vector for bits "0..length-1".

    BitVec(BitVec const& v);
    // effects  Create copy of "v".

    BitVec& operator = (BitVec const& v);
    // requires "size() == v.size()"
    // modifies "this"
    // effects  Copy contents of "v" into "this"

    ~BitVec();
    // effects  Release all storage for bit vector

    int length() const;
    // effects  Return bitvector length

    int empty() const;
    // effects  Return true iff no bits are set in entire vector

    int get(int i) const;
    // requires "i >= 0" and "i < length()"
    // effects  Returns true iff bit vector contains "i".

    void set(int i);
    // requires "i >= 0" and "i < length()"
    // modifies "this"
    // effects  Insert bit "i" into vector.

    void clear(int i);
    // requires "i >= 0" and "i < length()"
    // modifies "this"
    // effects  Remove bit "i" from vector.

    void clear();
    // modifies "this"
    // effects  Removes all bits from vector.

    void invert();
    // modifies "this"
    // effects  foreach i in [0..length()-1]
    //              post(value(i)) == !pre(value(i))

    int first_set() const;
    // effects  Return number of smallest bit contained in bit vector.
    //          If bit vector is empty, then returns "length()".

    int first_clear() const;
    // effects  Return number of smallest bit not contained in bit vector.
    //          If bit vector is full, then returns "length()".
  private:
    void init(int l);
    // Initialize to length "l".

    int len;            // Number of bits in vector
    int alloc;          // Number of allocated words

    bitvword* word;     // Word array

    int word_size() const;              // Number of bits per word
    int word_numb(int n) const;         // Number of word containing bit "n"
    int word_offs(int n) const;         // Offset of bit "n" in its word
    bitvword word_mask(int n) const;    // Mask for extracting bit "n"
};

#define BITVEC_WSIZE  32
#define BITVEC_WSHIFT 5

// If longs are actually bigger, exploit that fact.
#ifdef __alpha
#undef BITVEC_WSIZE
#undef BITVEC_WSHIFT
#define BITVEC_WSIZE  64
#define BITVEC_WSHIFT 6
#endif

inline int BitVec::word_size() const       { return BITVEC_WSIZE; }
inline int BitVec::word_numb(int n) const  { return (n >> BITVEC_WSHIFT); }
inline int BitVec::word_offs(int n) const  { return (n & (BITVEC_WSIZE-1)); }
inline bitvword BitVec::word_mask(int n) const {
    return (((bitvword) 1) << word_offs(n));
}

inline int BitVec::length() const {
    return len;
}

inline int BitVec::get(int i) const {
    assert((i >= 0) && (i < len));
    return (word[word_numb(i)] & word_mask(i)) ? 1 : 0;
}

inline void BitVec::set(int i) {
    assert((i >= 0) && (i < len));
    word[word_numb(i)] |= word_mask(i);
}

inline void BitVec::clear(int i) {
    assert((i >= 0) && (i < len));
    word[word_numb(i)] &= ~word_mask(i);
}

#endif /* _BITVEC_H */
