/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _UID_H
#define _UID_H

#include "hashfuncs.h"
#include "ohashset.h"

extern char const* uid_new();
    // effects - Return new uid.  The returned string is allocated
    //           via new char[...] and should be deleted by the caller
    //           when the caller is done using it.

// Set of uids
declareOpenHashSet(UidSet,char const*,hash_string,cmp_string)

#endif /* _UID_H */
