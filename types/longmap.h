/* Copyright (c) 1996  by Sanjay Ghemawat */
#ifndef _LONGMAP_H
#define _LONGMAP_H

#include <limits.h>

// Special table representation that encodes tags as small negative numbers
class LongMapRep {
  private:
    struct Entry {
	long	key;
	long	val;
    };
    Entry* list;
  public:
    LongMapRep(int entries) {
	list = new Entry[entries];
	for (int i = entries-1; i >= 0; i--) {
	    list[i].key = LONG_MIN;
	}
    }

    inline ~LongMapRep() {
	delete [] list;
    }

    inline long& key(int i) const	{ return list[i].key; }
    inline long& val(int i) const	{ return list[i].val; }
    inline int is_full(int i) const	{ return (list[i].key > LONG_MIN+1); }
    inline int is_empty(int i) const	{ return (list[i].key == LONG_MIN); }
    inline int is_del(int i) const	{ return (list[i].key == LONG_MIN+1); }
    inline void clear(int i)		{ list[i].key = LONG_MIN; }
    inline void kill(int i)		{ list[i].key = LONG_MIN+1; }
    inline void store(int i, long k)	{ list[i].key = k; }
};

#define HTABLE LongMap
#define HKEY long
#define HVAL long
#define HASHER(x) (x)
#define HTABLEREP LongMapRep
#include "htable.h"

#endif /* _LONGMAP_H */
