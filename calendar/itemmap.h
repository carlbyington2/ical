/* Copyright (c) 1995 by Sanjay Ghemawat */

#ifndef _ITEMMAP_H
#define _ITEMMAP_H

#include "item.h"

// An ItemMap maps from an item to its uid.  We use an array of
// "Item*" for its representation.  The key for a given item is
// its uid.  The value is the item itself.

class ItemMapRep {
  private:
    // Empty entries are 0, deleted entries are -1
    Item** list;
  public:
    ItemMapRep(int entries) {
        list = new Item*[entries];
        memset(list, 0, sizeof(Item*) * entries);
    }

    inline ~ItemMapRep() {
        delete [] list;
    }

    inline char const* key(int i) const         { return list[i]->GetUid(); }
    inline Item*&      val(int i) const         { return list[i]; }

    inline int is_full(int i) const {
        return ((list[i] != 0) && ((list[i] != ((Item*)-1))));
    }

    inline int is_empty(int i) const {
        return (list[i] == 0);
    }

    inline int is_del(int i) const {
        return (list[i] == ((Item*)-1));
    }

    inline void clear(int i) {
        list[i] = 0;
    }

    inline void kill(int i) {
        list[i] = (Item*)-1;
    }

    inline void store(int i, char const* key) {
        // No need to do anything to store "key".  When the value
        // is set for this slot, the key will also be automatically set.
    }
};

// Implement a map from uids to items
#define HTABLE          ItemMap
#define HKEY            char const*
#define HVAL            Item*
#define HASHER          hash_string
#define HEQUAL          cmp_string
#define HTABLEREP       ItemMapRep
#include "htable.h"

#endif /* _ITEMMAP_H */
