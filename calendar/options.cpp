/* Copyright (c) 1994 Sanjay Ghemawat */
#include <assert.h>
#include "basic.h"
#include "options.h"
#include "arrays.h"
#include "lexer.h"
#include "misc.h"

OptionMap::~OptionMap() {
    for (OptionMap_Bindings b = rep; b.ok(); b.next()) {
        delete [] ((char*) b.key());
        delete [] ((char*) b.val());
    }
    delete rep;
}

void OptionMap::store(char const* key, char const* value) {
    char const* old_value;
    if (rep->fetch(key, old_value)) {
        // Replace old value with new
        rep->store(key, copy_string(value));
        delete [] ((char*) old_value);
    }
    else {
        // Insert new value
        rep->store(copy_string(key), copy_string(value));
    }
}

// XXX This is a memory leak of the key/value pair from the option map
void OptionMap::remove(char const* key) {
    rep->remove(key);
}

void OptionMap::write(charArray* out) const {
    // Generated sorted list
    pointerArray list;
    for (OptionMap_Bindings b = rep; b.ok(); b.next())
        list.append((void*) b.key());

    // This is slow, but the option list should be fairly small
    int i = 0;
    int num = list.size();
    while (i < num) {
        /* Find min element in list[i..] */
        int minIndex = i;
        for (int j = i+1; j < num; j++) {
            if (strcmp((char const*)list[j],(char const*)list[minIndex]) < 0) {
                minIndex = j;
            }
        }

        void* temp;
        temp = list[i];
        list[i] = list[minIndex];
        list[minIndex] = temp;

        i++;
    }

    // Now print out in sorted order
    for (i = 0; i < num; i++) {
        char const* key = (char const*) list[i];
        assert(rep->contains(key));

        char const* val;
        rep->fetch(key, val);
        append_string(out, key);
        append_string(out, " [");
        Lexer::PutString(out, val);
        append_string(out, "]\n");
    }
}

implementOpenHashMap(OptionMapRep,char const*,char const*,hash_string,cmp_string)
