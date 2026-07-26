/* Copyright (c) 1994 Sanjay Ghemawat */
#ifndef _OPTIONS_H
#define _OPTIONS_H

// Option map
//
// This class adds appropriate allocation/de-allocation of strings
// to a raw hash map.

#include <string.h>
#include "basic.h"
#include "hashfuncs.h"
#include "ohashmap.h"

declareOpenHashMap(OptionMapRep,char const*,char const*,hash_string,cmp_string)

class charArray;
typedef OptionMapRep_Bindings OptionMap_Bindings;

class OptionMap {
  public:
    OptionMap();
    ~OptionMap();

    bool contains(char const* key) const;
    bool fetch(char const* key, char const*& value) const;
    void store(char const* key, char const* value);
    void remove(char const* key);

    OptionMap_Bindings bindings() const;
    void write(charArray*) const;
  private:
    OptionMapRep* rep;
};

inline OptionMap::OptionMap() {
    rep = new OptionMapRep;
}

inline bool OptionMap::contains(char const* key) const {
    return (rep->contains(key));
}

inline bool OptionMap::fetch(char const* key, char const*& value) const {
    return (rep->fetch(key, value));
}

inline OptionMap_Bindings OptionMap::bindings() const {
    OptionMap_Bindings b = rep;
    return b;
}

#endif /* _OPTIONS_H */
