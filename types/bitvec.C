/* Copyright (c) 1995 by Sanjay Ghemawat */
#include <assert.h>
#include "basic.h"
#include "bitvec.h"

void BitVec::init(int l) {
    assert(l >= 0);

    len = l;
    alloc = word_numb(l-1)+1;
    word = new bitvword[alloc];
    for (int i = 0; i < alloc; i++)
        word[i] = 0;
}

BitVec::BitVec(int l) {
    init(l);
}

BitVec::BitVec(BitVec const& v) {
    init(v.len);
    *this = v;
}

BitVec& BitVec::operator = (BitVec const& v) {
    assert(len == v.len);

    for (int i = 0; i < alloc; i++)
        word[i] = v.word[i];
    return *this;
}

BitVec::~BitVec() {
    delete [] word;
}

void BitVec::clear() {
    for (int i = 0; i < alloc; i++)
        word[i] = 0;
}

void BitVec::invert() {
    for (int i = 0; i < alloc; i++)
        word[i] = ~word[i];

    if (alloc > 0) {
        // Adjust the last word so that all out of range bits are zero.
        bitvword last = word[alloc-1];  // The last word
        int boundary = word_offs(len);  // First out of range bit index
        if (boundary > 0) {
            // We just get the bit mask for the first out of range bit
            // and subtract one from it to get a mask that corresponds to
            // all valid bits in the word.
            word[alloc-1] = last & (word_mask(boundary) - 1);
        }
    }    
}

int BitVec::empty() const {
    for (int i = 0; i < alloc; i++)
        if (word[i] != 0) return 0;
    return 1;
}

int BitVec::first_set() const {
    int num = word_numb(len-1);

    for (int i = 0; i <= num; i++) {
        bitvword w = word[i];
        if (w == 0) continue;

        // Look in this word
        int result = i * word_size();
        while (1) {
            if (result >= len) return len;
            if (w & 1) return result;
            result++;
            w >>= 1;
        }
    }
    return len;
}

int BitVec::first_clear() const {
    int num = word_numb(len-1);

    for (int i = 0; i <= num; i++) {
        bitvword w = ~word[i];
        if (w == 0) continue;

        int result = i * word_size();
        while (1) {
            if (result >= len) return len;
            if (w & 1) return result;
            result++;
            w >>= 1;
        }
    }
    return len;
}
