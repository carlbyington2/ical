/* Copyright (c) 1995 by Sanjay Ghemawat */
#ifndef _INTSET_H
#define _INTSET_H

#define HTABLE     IntSet
#define HKEY       int
#define HASHER(x)  (x)
#define HTABLEREP  StructTable1<int>
#include "htable.h"

#endif /* _INTSET_H */
