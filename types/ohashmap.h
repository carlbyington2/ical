/* Copyright (c) 1995 by Sanjay Ghemawat */
#ifndef _OHASHMAP_H
#define _OHASHMAP_H


/*
 * Generic open hash map.
 *
 * You can declare a map from type <Key> to type <Value> and name the
 * resulting class <MapClass> by saying --
 *
 *      declareOpenHashMap(<MapClass>, <Key>, <Value>, <Hash>, <Compare>)
 *
 * You should provide a corresponding implementation of <MapClass> in
 * exactly one .cc file --
 *
 *      implementOpenHashMap(<MapClass>, <Key>, <Value>, <Hash>, <Compare>)
 *
 * <Hash> should be the name of a function (or a macro) that can be
 * supplied a <Key> as its single parameter.  It should return a
 * non-negative integer.
 *
 * <Comparer> should be the name of a function (or a macro) that can be
 * supplied two <Keys>.  It should return TRUE iff the supplied keys
 * are equal.
 */

#include <assert.h>
#include <stdio.h>
#include "basic.h"

#define HASHMAP_PRECBITS ((sizeof(int) << 3) - 2)
#define HASHMAP_PRECMASK ((1 << HASHMAP_PRECBITS) - 1)

#define declareOpenHashMap(HashMap,Key,Val,hasher,comparer)                   \
                                                                              \
class HashMap##_Bindings;                                                     \
                                                                              \
class HashMap {                                                               \
  public:                                                                     \
    HashMap();                                                                \
    /*                                                                        \
     * effects - Create empty map.                                            \
     */                                                                       \
                                                                              \
    HashMap(int predict_count);                                               \
    /*                                                                        \
     * requires - predict_count >= 0                                          \
     * effects  - Create empty map.                                           \
     *                                                                        \
     * The created map will be sized to allow fast growth to predict_count    \
     *                                                                        \
     * entries.                                                               \
     */                                                                       \
                                                                              \
    HashMap(HashMap const& m);                                                \
    /*                                                                        \
     * effects  - Create new map with same contents as "m".                   \
     */                                                                       \
                                                                              \
    HashMap& operator=(HashMap const& m);                                     \
    /*                                                                        \
     * effects  - Replace contents of map with the contents of "m".           \
     */                                                                       \
                                                                              \
    ~HashMap();                                                               \
    /*                                                                        \
     * effects - Destroy storage for map.                                     \
     */                                                                       \
                                                                              \
    int size() const;                                                         \
                                                                              \
    bool contains(Key k) const;                                               \
    /*                                                                        \
     * effects - Return true iff map has binding for k                        \
     */                                                                       \
                                                                              \
    bool fetch(Key k, Val& v) const;                                          \
    /*                                                                        \
     * modifies - v                                                           \
     * effects  - If map has binding for k, post(v) = value bound to k        \
     *            and returns TRUE.  Else post(v) = pre(v) and returns FALSE. \
     */                                                                       \
                                                                              \
    Val fetch(Key k) const;                                                   \
    /*                                                                        \
     * requires - Map contains binding for k.                                 \
     * effects  - Return value from binding for k.                            \
     */                                                                       \
                                                                              \
    void store(Key k, Val v);                                                 \
    /*                                                                        \
     * modifies - this                                                        \
     * effects  - Binding (k->v) is added to this.  Any previous bindings for \
     *            k are removed.                                              \
     */                                                                       \
                                                                              \
    void remove(Key k);                                                       \
    /*                                                                        \
     * modifies - this                                                        \
     * effects  - Any binding for k is removed from this.                     \
     */                                                                       \
                                                                              \
    void clear();                                                             \
    /*                                                                        \
     * modifies - this                                                        \
     * effects  - All bindings are removed from this.                         \
     */                                                                       \
                                                                              \
    void reclaim();                                                           \
    /*                                                                        \
     * effects  - Reduce memory use if possible.                              \
     */                                                                       \
                                                                              \
    void check();                                                             \
    /*                                                                        \
     * effects  - Check various rep invariants.  Die on error.                \
     */                                                                       \
                                                                              \
  private:                                                                    \
    friend class HashMap##_Bindings;                                          \
                                                                              \
    enum Typ { empty, occupied, obsolete };                                   \
                                                                              \
    int  count;                                                               \
    int  deletecount;                                                         \
    int  tsize;                                                               \
    int  mask;                                                                \
    int  slotBits;                                                            \
    Key* key;                                                                 \
    Val* val;                                                                 \
    Typ* typ;                                                                 \
                                                                              \
    enum { empty_fraction = 1 };                                              \
                                                                              \
    /*                                                                        \
     * Rep Invariant                                                          \
     *                                                                        \
     * * key, val, typ are arrays                                             \
     * * key, val, typ have length "tsize".                                   \
     * * (typ[i] == typ[j] == occupied and key[i] == key[j]) => (i == j)      \
     *   I.e., no duplicate entries for any key.                              \
     * * Key k is stored at index (hash(k) + (j*hash2(hash(k)))) % tsize      \
     *   where for all i s.t. 0 <= i < j,                                     \
     *          typ[(hash(k) + i*hash2(hash(k))) % tsize] != empty            \
     *   I.e., if we search for key k starting at index hash(k, tsize)        \
     *   and advancing by hash2(k) at every turn, and if k is present in      \
     *   the table, then all buckets hit before we find k will be non-empty.  \
     *   This portion of the rep-invariant allows us to stop the search       \
     *   as soon as we hit an empty slot.                                     \
     * * count = number of i s.t. typ[i] == occupied.                         \
     * * deletecount = number if i s.t. typ[i] == obsolete.                   \
     * * tsize is usually the smallest power of 2 s.t.                        \
     *          tsize > count+deletecount                                     \
     *          tsize >= empty_fraction*(count+deletecount)                   \
     *   I.e, there is always at least one free entry and at most             \
     *   1/empty_fraction of the table is full.                               \
     * * mask = tsize-1 (mask is used for fast arithmetic modulo tsize)       \
     * * slotBits = lg(tsize) (used for fast hash value extraction)           \
     */                                                                       \
                                                                              \
    /*                                                                        \
     * Abstraction Function                                                   \
     *                                                                        \
     * A() = { <key[i], val[i]> | (0 < i < tsize) and (typ[i] == occupied) }  \
     */                                                                       \
                                                                              \
    /*                                                                        \
     * Internal Operations                                                    \
     */                                                                       \
                                                                              \
    void init(int s);                                                         \
    /*                                                                        \
     * effects  - ignores old state and sets new empty state to hold at       \
     *            least "s" elements.                                         \
     */                                                                       \
                                                                              \
    void resize();                                                            \
    /*                                                                        \
     * effects  - Table is resized.                                           \
     */                                                                       \
                                                                              \
    int find_index(Key k) const;                                              \
    /*                                                                        \
     * effects  - Return index for key k.  If map contains binding            \
     *            for k, then the index of that binding is returned.          \
     *            Otherwise, the first empty index in hash search for         \
     *            k is returned.                                              \
     */                                                                       \
                                                                              \
    static int hash2(int x);                                                  \
    /*                                                                        \
     * effects  - Return hash value for x.                                    \
     */                                                                       \
                                                                              \
    static const int prime_table[13];                                         \
};                                                                            \
                                                                              \
/*                                                                            \
 * Generate bindings                                                          \
 */                                                                           \
class HashMap##_Bindings {                                                    \
  public:                                                                     \
    HashMap##_Bindings(HashMap const* table);                                 \
    /*                                                                        \
     * requires - table is not modified for the lifetime of the generator     \
     * effects  - Generate each binding in table, exactly once.               \
     *        Bindings may be generated in arbitrary order.                   \
     */                                                                       \
                                                                              \
    bool ok() const;                                                          \
    Key  key() const;                                                         \
    Val  val() const;                                                         \
    void del();                                                               \
    void next();                                                              \
  private:                                                                    \
    HashMap const* table;                                                     \
    int index;                                                                \
                                                                              \
    void skip_empty();                                                        \
};                                                                            \
                                                                              \
inline HashMap::HashMap() {                                                   \
    init(1);                                                                  \
}                                                                             \
                                                                              \
inline HashMap::~HashMap() {                                                  \
    delete [] key;                                                            \
    delete [] val;                                                            \
    delete [] typ;                                                            \
}                                                                             \
                                                                              \
inline int HashMap::size() const {                                            \
    return count;                                                             \
}                                                                             \
                                                                              \
inline bool HashMap::contains(Key k) const {                                  \
    int i = find_index(k);                                                    \
    return (typ[i] == occupied);                                              \
}                                                                             \
                                                                              \
inline bool HashMap::fetch(Key k, Val& v) const {                             \
    int i = find_index(k);                                                    \
    if (typ[i] == occupied) {                                                 \
        v = val[i];                                                           \
        return TRUE;                                                          \
    }                                                                         \
    else {                                                                    \
        return FALSE;                                                         \
    }                                                                         \
}                                                                             \
                                                                              \
inline Val HashMap::fetch(Key k) const {                                      \
    return val[find_index(k)];                                                \
}                                                                             \
                                                                              \
inline void HashMap::store(Key k, Val v) {                                    \
    int i = find_index(k);                                                    \
    if (typ[i] == occupied) {                                                 \
        /* Overwrite */                                                       \
        val[i] = v;                                                           \
    }                                                                         \
    else {                                                                    \
        /* New entry */                                                       \
        count++;                                                              \
        typ[i] = occupied;                                                    \
        key[i] = k;                                                           \
        val[i] = v;                                                           \
                                                                              \
        int used = count+deletecount;                                         \
        if ((tsize <= used) || (tsize < (empty_fraction*used))) {             \
            resize();                                                         \
        }                                                                     \
    }                                                                         \
}                                                                             \
                                                                              \
inline void HashMap::remove(Key k) {                                          \
    int i = find_index(k);                                                    \
    if (typ[i] == occupied) {                                                 \
        /* Remove */                                                          \
        count--;                                                              \
        deletecount++;                                                        \
        typ[i] = obsolete;                                                    \
                                                                              \
        if (tsize > 3*empty_fraction*count) {                                 \
            resize();                                                         \
        }                                                                     \
    }                                                                         \
}                                                                             \
                                                                              \
inline void HashMap::clear() {                                                \
    for (int i = 0; i < tsize; i++)                                           \
        typ[i] = empty;                                                       \
    count = 0;                                                                \
    deletecount = 0;                                                          \
}                                                                             \
                                                                              \
inline void HashMap::reclaim() {                                              \
    if (tsize > empty_fraction*count)                                         \
        resize();                                                             \
}                                                                             \
                                                                              \
inline int HashMap::hash2(int x) {                                            \
    return prime_table[x % 13];                                               \
}                                                                             \
                                                                              \
inline HashMap##_Bindings::HashMap##_Bindings(HashMap const* t) {             \
    table = t;                                                                \
    index = 0;                                                                \
    skip_empty();                                                             \
}                                                                             \
                                                                              \
inline bool HashMap##_Bindings::ok() const {                                  \
    return (index < table->tsize);                                            \
}                                                                             \
                                                                              \
inline Key HashMap##_Bindings::key() const {                                  \
    return table->key[index];                                                 \
}                                                                             \
                                                                              \
inline Val HashMap##_Bindings::val() const {                                  \
    return table->val[index];                                                 \
}                                                                             \
                                                                              \
inline void HashMap##_Bindings::del() {                                       \
    ((HashMap*) table)->count--;                                              \
    ((HashMap*) table)->deletecount++;                                        \
    ((HashMap*) table)->typ[index] = HashMap::obsolete;                       \
}                                                                             \
                                                                              \
inline void HashMap##_Bindings::next() {                                      \
    index++;                                                                  \
    skip_empty();                                                             \
}                                                                             \

#define implementOpenHashMap(HashMap,Key,Val,hasher,comparer)                 \
                                                                              \
HashMap::HashMap(int c) {                                                     \
    init(c);                                                                  \
}                                                                             \
                                                                              \
void HashMap::init(int s) {                                                   \
    /* Search for the smallest power of 2 > s and >= "empty_fraction * s" */  \
                                                                              \
    int power = 1;                                                            \
    while ((power <= s) || (power < empty_fraction*s))                        \
        power = power << 1;                                                   \
    s = power;                                                                \
                                                                              \
    count = 0;                                                                \
    deletecount = 0;                                                          \
    tsize = s;                                                                \
    mask = s-1;                                                               \
    slotBits = 0;                                                             \
    int x = s;                                                                \
    while (x > 1) {                                                           \
        x >>= 1;                                                              \
        slotBits++;                                                           \
    }                                                                         \
    key = new Key[s];                                                         \
    val = new Val[s];                                                         \
    typ = new Typ[s];                                                         \
    for (int i = 0; i < s; i++) {                                             \
        typ[i] = empty;                                                       \
    }                                                                         \
}                                                                             \
                                                                              \
HashMap::HashMap(HashMap const& m) {                                          \
    init(m.size());                                                           \
                                                                              \
    for (int i = 0; i < m.tsize; i++)                                         \
        if (m.typ[i] == occupied) store(m.key[i], m.val[i]);                  \
}                                                                             \
                                                                              \
HashMap& HashMap::operator = (HashMap const& m) {                             \
    /* Make sure we handle "x = x" correctly. */                              \
    if (this == &m) return *this;                                             \
                                                                              \
    delete [] key;                                                            \
    delete [] val;                                                            \
    delete [] typ;                                                            \
    init(m.size());                                                           \
                                                                              \
    for (int i = 0; i < m.tsize; i++)                                         \
        if (m.typ[i] == occupied) store(m.key[i], m.val[i]);                  \
                                                                              \
    return *this;                                                             \
}                                                                             \
                                                                              \
/*                                                                            \
 * Table for secondary hash function.                                         \
 *                                                                            \
 * Important points --                                                        \
 *  * The entries are prime numbers so that the search process of             \
 *    incrementing the index by the secondary hash value modulo a             \
 *    power of 2 will end up scanning the entire table.                       \
 *  * The size of the table is a prime number to differentiate it from        \
 *    the primary hash function, which is used modulo a power of 2.           \
 */                                                                           \
                                                                              \
const int HashMap::prime_table[13] = {                                        \
    3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43                           \
};                                                                            \
                                                                              \
void HashMap::resize() {                                                      \
    /*                                                                        \
     * Save away old contents                                                 \
     */                                                                       \
    Typ* old_typ = typ;                                                       \
    Key* old_key = key;                                                       \
    Val* old_val = val;                                                       \
    int  old_size = tsize;                                                    \
                                                                              \
    init(count);                                                              \
                                                                              \
    /*                                                                        \
     * Restore old contents                                                   \
     */                                                                       \
    for (int i = 0; i < old_size; i++) {                                      \
        if (old_typ[i] == occupied) {                                         \
            store(old_key[i], old_val[i]);                                    \
        }                                                                     \
    }                                                                         \
                                                                              \
    /*                                                                        \
     * Free storage                                                           \
     */                                                                       \
    delete [] old_key;                                                        \
    delete [] old_val;                                                        \
    delete [] old_typ;                                                        \
}                                                                             \
                                                                              \
int HashMap::find_index(Key k) const {                                        \
    int h1 = hasher(k);                                                       \
    int first = (h1 & HASHMAP_PRECMASK) >> (HASHMAP_PRECBITS - slotBits);     \
                                                                              \
    /*                                                                        \
     * First try without computing secondary hash function                    \
     */                                                                       \
    Typ t = typ[first];                                                       \
    if ((t == empty) || ((t == occupied) && (comparer(key[first],k)))) {      \
        return first;                                                         \
    }                                                                         \
                                                                              \
    /*                                                                        \
     * Now scan through, incrementing the index by the secondary hash value   \
     * on every iteration.                                                    \
     */                                                                       \
                                                                              \
    int h2 = hash2(h1);                                                       \
    int i = (first + h2) & mask;                                              \
                                                                              \
    int xcount = 1;                                                           \
    while (1) {                                                               \
        t = typ[i];                                                           \
        if ((t == empty) || ((t == occupied) && (comparer(key[i],k)))) {      \
            return i;                                                         \
        }                                                                     \
        i = (i + h2) & mask;                                                  \
        xcount++;                                                             \
        if (xcount > tsize) {                                                 \
            fprintf(stderr, "hash loop\n");                                   \
            fprintf(stderr, "   h1 = %d\n", h1);                              \
            fprintf(stderr, "   h2 = %d\n", h2);                              \
            fprintf(stderr, "   sz = %d\n", tsize);                           \
            fprintf(stderr, "   ct = %d\n", count);                           \
            assert(0);                                                        \
        }                                                                     \
    }                                                                         \
}                                                                             \
                                                                              \
void HashMap::check() {                                                       \
    int i, j;                                                                 \
                                                                              \
    /* Check for duplicates */                                                \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] != occupied) continue;                                     \
                                                                              \
        for (j = i+1; j < tsize; j++) {                                       \
            if (typ[j] == occupied) {                                         \
                assert(! comparer(key[i], key[j]));                           \
            }                                                                 \
        }                                                                     \
    }                                                                         \
                                                                              \
    /* Check that hash values for all available keys do not wrap */           \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] != occupied) continue;                                     \
                                                                              \
        bool* marked = new bool[tsize];                                       \
        for (j = 0; j < tsize; j++) {                                         \
            marked[j] = FALSE;                                                \
        }                                                                     \
                                                                              \
        int h1 = hasher(key[i]);                                              \
        int h2 = hash2(h1);                                                   \
        int first = (h1 & HASHMAP_PRECMASK) >> (HASHMAP_PRECBITS - slotBits); \
                                                                              \
        for (j = 0; j < tsize; j++) {                                         \
            int index = (first + h2*j) & mask;                                \
            assert(! marked[index]);                                          \
            marked[index] = TRUE;                                             \
        }                                                                     \
                                                                              \
        delete [] marked;                                                     \
    }                                                                         \
                                                                              \
    /* Check that there are no empty slots in hash sequence for each key */   \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] != occupied) continue;                                     \
                                                                              \
        int h1 = hasher(key[i]);                                              \
        int h2 = hash2(h1);                                                   \
        int first = (h1 & HASHMAP_PRECMASK) >> (HASHMAP_PRECBITS - slotBits); \
                                                                              \
        /* Loop at most tsize times */                                        \
        for (j = 0; j < tsize; j++) {                                         \
            int index = (first + h2*j) & mask;                                \
            /* Check for hole */                                              \
            assert(typ[index] != empty);                                      \
                                                                              \
            if (index == i) break;                                            \
        }                                                                     \
    }                                                                         \
                                                                              \
    /* count */                                                               \
    j = 0;                                                                    \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] == occupied) j++;                                          \
    }                                                                         \
    assert(count == j);                                                       \
                                                                              \
    /* deletecount */                                                         \
    j = 0;                                                                    \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] == obsolete) j++;                                          \
    }                                                                         \
    assert(deletecount == j);                                                 \
                                                                              \
    /* tsize is power of two */                                               \
    i = 1;                                                                    \
    while (i < tsize) {                                                       \
        i = i << 1;                                                           \
    }                                                                         \
    assert(i == tsize);                                                       \
                                                                              \
    /* Check that at least one slot is empty */                               \
    assert(tsize > count+deletecount);                                        \
                                                                              \
    /* mask */                                                                \
    assert(mask == (tsize - 1));                                              \
                                                                              \
    /* slotBits */                                                            \
    assert(tsize == (1 << slotBits));                                         \
}                                                                             \
                                                                              \
void HashMap##_Bindings::skip_empty() {                                       \
    while ((index < table->tsize) &&                                          \
           (table->typ[index] != HashMap::occupied)) {                        \
        index++;                                                              \
    }                                                                         \
}                                                                             \

#endif /* _OHASHMAP_H */
