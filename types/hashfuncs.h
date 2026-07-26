/* Copyright (c) 1995 by Sanjay Ghemawat */
#ifndef _HASHFUNCS_H
#define _HASHFUNCS_H

// Some common hash functions!
#include <string.h>

extern int hash_int(int);
    // effects  Uses multiplicative hashing to construct a nice hash
    //          value from an integer.

extern int hash_ptr(void*);
    // effects  Uses multiplicative hashing on pointer values.
    //          Do not use this macro in an environment where a garbage
    //          collector may move objects around.

extern int hash_string(char const*);
    // effects  Hashes from a string to an integer.

extern int hash_string_lowercase(char const*);
    // effects  Converts string to lowercase before extracting a hash value.
    //          This ensures that if two strings are identical except for
    //          case, then they will map to the same hash value.  This
    //          behavior is useful for case insensitive sets and tables.

// Related comparison functions because they are most often used in
// hash tables.

#define cmp_int(a,b)    ((a) == (b))
#define cmp_ptr(a,b)    ((a) == (b))
#define cmp_string(a,b) (strcmp((a),(b)) == 0)
#define cmp_string_lowercase(a,b) (strcasecmp((a),(b)) == 0)

#endif /* _HASHFUNCS_H */
