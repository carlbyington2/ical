/* Copyright (c) 1995 by Sanjay Ghemawat */
#include <ctype.h>
#include "hashfuncs.h"

/*
 * Multiplicative hash function.
 *
 * The value "1737350766" is the bottom 32 bits of int(PHI * 2^30),
 * suggested in CLR, Knuth.  The hash value should be top "k" of the
 * bottom 30 bits of the number multipled by this value.
 */

int hash_int(int x) {
    /*
     * Multiply by a constant and take the bottom bits of the
     * result.
     */

    return (int) ((((unsigned int) x) * 1737350766) & 0x7fffffff);
}

int hash_ptr(void* x) {
    return hash_int((int) ((unsigned long) x));
}

/* String hashing out of The Dragon Book. */
int hash_string(char const* x) {
    char const* p;
    unsigned int h = 0;
    unsigned int g;

    for (p = x; *p != '\0'; p++) {
        h = (h << 4) + *p;
        if ((g = h & 0xf0000000)) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return (int) h;
}

int hash_string_lowercase(char const* x) {
    char const* p;
    unsigned int h = 0;
    unsigned int g;

    for (p = x; *p != '\0'; p++) {
        h = (h << 4) + tolower(*p);
        if ((g = h & 0xf0000000)) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return (int) h;
}
