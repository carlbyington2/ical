/* Copyright (c) 1995 by Sanjay Ghemawat */
#ifndef _OHASHSET_H
#define _OHASHSET_H


/*
 * Generic open hash set.
 *
 * You can declare a set of type <Element> and name the
 * resulting class <SetClass> by saying --
 *
 *      declareOpenHashSet(<SetClass>, <Element>, <Hash>, <Compare>)
 *
 * You should provide a corresponding implementation of <SetClass> in
 * exactly one .cc file --
 *
 *      implementOpenHashSet(<SetClass>, <Element>, <Hash>, <Compare>)
 *
 * <Hash> should be the name of a function (or a macro) that can be
 * supplied a <Element> as its single parameter.  It should return a
 * non-negative integer.
 *
 * <Comparer> should be the name of a function (or a macro) that can be
 * supplied two <Elements>.  It should return TRUE iff the supplied
 * elements are equal.
 */

#include <assert.h>
#include <stdio.h>
#include "basic.h"

#define HASHSET_PRECBITS ((sizeof(int) << 3) - 2)
#define HASHSET_PRECMASK ((1 << HASHSET_PRECBITS) - 1)

#define declareOpenHashSet(HashSet,Elt,hasher,comparer)                       \
                                                                              \
class HashSet##_Elements;                                                     \
                                                                              \
class HashSet {                                                               \
  public:                                                                     \
    HashSet();                                                                \
    /*                                                                        \
     * effects - Create empty set.                                            \
     */                                                                       \
                                                                              \
    HashSet(int predict_count);                                               \
    /*                                                                        \
     * requires - predict_count >= 0                                          \
     * effects  - Create empty set.                                           \
     *                                                                        \
     * The created set will be sized to allow fast growth to predict_count    \
     *                                                                        \
     * entries.                                                               \
     */                                                                       \
                                                                              \
    HashSet(HashSet const& s);                                                \
    /*                                                                        \
     * effects  - Create new set with the same contents as "s".               \
     */                                                                       \
                                                                              \
    HashSet& operator=(HashSet const& s);                                     \
    /*                                                                        \
     * effects  - Replace contents of set with the contents of "s".           \
     */                                                                       \
                                                                              \
    ~HashSet();                                                               \
    /*                                                                        \
     * effects - Destroy storage for set.                                     \
     */                                                                       \
                                                                              \
    int size() const;                                                         \
                                                                              \
    bool contains(Elt k) const;                                               \
    /*                                                                        \
     * effects - Return true iff set contains k                               \
     */                                                                       \
                                                                              \
    void insert(Elt k);                                                       \
    /*                                                                        \
     * modifies - this                                                        \
     * effects  - k is added to this if this did not contain in already.      \
     */                                                                       \
                                                                              \
    void remove(Elt k);                                                       \
    /*                                                                        \
     * modifies - this                                                        \
     * effects  - Remove k if set contains k.                                 \
     */                                                                       \
                                                                              \
    void clear();                                                             \
    /*                                                                        \
     * modifies - this                                                        \
     * effects  - All elements are removed from this.                         \
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
  private:                                                                    \
    friend class HashSet##_Elements;                                          \
                                                                              \
    enum Typ { empty, occupied, obsolete };                                   \
                                                                              \
    int  count;                                                               \
    int  deletecount;                                                         \
    int  tsize;                                                               \
    int  mask;                                                                \
    int  slotBits;                                                            \
    Elt* elt;                                                                 \
    Typ* typ;                                                                 \
                                                                              \
    enum { empty_fraction = 1 };                                              \
                                                                              \
    /*                                                                        \
     * Rep Invariant                                                          \
     *                                                                        \
     * * elt, typ are arrays                                                  \
     * * elt, typ have length "tsize".                                        \
     * * (typ[i] == typ[j] == occupied and elt[i] == elt[j]) => (i == j)      \
     *   I.e., no duplicate entries for any elt.                              \
     * * Elt k is stored at index (hash(k) + (j*hash2(hash(k)))) % tsize      \
     *   where for all i s.t. 0 <= i < j,                                     \
     *          typ[(hash(k) + i*hash2(hash(k))) % tsize] != empty            \
     *   I.e., if we search for elt k starting at index hash(k, tsize)        \
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
     * A() = { elt[i] | (0 < i < tsize) and (typ[i] == occupied) }            \
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
    int find_index(Elt k) const;                                              \
    /*                                                                        \
     * effects  - Return index for elt k.  If set contains k                  \
     *            then the index for k is returned.                           \
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
class HashSet##_Elements {                                                    \
  public:                                                                     \
    HashSet##_Elements(HashSet const* table);                                 \
    /*                                                                        \
     * requires - table is not modified for the lifetime of the generator     \
     * effects  - Generate each element in table, exactly once.               \
     *        Elements may be generated in arbitrary order.                   \
     */                                                                       \
                                                                              \
    bool ok() const;                                                          \
    Elt  get() const;                                                         \
    void del();                                                               \
    void next();                                                              \
  private:                                                                    \
    HashSet const* table;                                                     \
    int index;                                                                \
                                                                              \
    void skip_empty();                                                        \
};                                                                            \
                                                                              \
inline HashSet::HashSet() {                                                   \
    init(1);                                                                  \
}                                                                             \
                                                                              \
inline HashSet::~HashSet() {                                                  \
    delete [] elt;                                                            \
    delete [] typ;                                                            \
}                                                                             \
                                                                              \
inline int HashSet::size() const {                                            \
    return count;                                                             \
}                                                                             \
                                                                              \
inline bool HashSet::contains(Elt k) const {                                  \
    int i = find_index(k);                                                    \
    return (typ[i] == occupied);                                              \
}                                                                             \
                                                                              \
inline void HashSet::insert(Elt k) {                                          \
    int i = find_index(k);                                                    \
    if (typ[i] != occupied) {                                                 \
        /* New entry */                                                       \
        count++;                                                              \
        typ[i] = occupied;                                                    \
        elt[i] = k;                                                           \
                                                                              \
        int used = count+deletecount;                                         \
        if ((tsize <= used) || (tsize < (empty_fraction*used))) {             \
            resize();                                                         \
        }                                                                     \
    }                                                                         \
}                                                                             \
                                                                              \
inline void HashSet::remove(Elt k) {                                          \
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
inline void HashSet::clear() {                                                \
    for (int i = 0; i < tsize; i++)                                           \
        typ[i] = empty;                                                       \
    count = 0;                                                                \
    deletecount = 0;                                                          \
}                                                                             \
                                                                              \
inline void HashSet::reclaim() {                                              \
    if (tsize > empty_fraction*count)                                         \
        resize();                                                             \
}                                                                             \
                                                                              \
inline int HashSet::hash2(int x) {                                            \
    return prime_table[x % 13];                                               \
}                                                                             \
                                                                              \
inline HashSet##_Elements::HashSet##_Elements(HashSet const* t) {             \
    table = t;                                                                \
    index = 0;                                                                \
    skip_empty();                                                             \
}                                                                             \
                                                                              \
inline bool HashSet##_Elements::ok() const {                                  \
    return (index < table->tsize);                                            \
}                                                                             \
                                                                              \
inline Elt HashSet##_Elements::get() const {                                  \
    return table->elt[index];                                                 \
}                                                                             \
                                                                              \
inline void HashSet##_Elements::del() {                                       \
    ((HashSet*) table)->count--;                                              \
    ((HashSet*) table)->deletecount++;                                        \
    ((HashSet*) table)->typ[index] = HashSet::obsolete;                       \
}                                                                             \
                                                                              \
inline void HashSet##_Elements::next() {                                      \
    index++;                                                                  \
    skip_empty();                                                             \
}                                                                             \

#define implementOpenHashSet(HashSet,Elt,hasher,comparer)                     \
                                                                              \
HashSet::HashSet(int c) {                                                     \
    init(c);                                                                  \
}                                                                             \
                                                                              \
void HashSet::init(int s) {                                                   \
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
    elt = new Elt[s];                                                         \
    typ = new Typ[s];                                                         \
    for (int i = 0; i < s; i++) {                                             \
        typ[i] = empty;                                                       \
    }                                                                         \
}                                                                             \
                                                                              \
HashSet::HashSet(HashSet const& s) {                                          \
    init(s.size());                                                           \
                                                                              \
    for (int i = 0; i < s.tsize; i++)                                         \
        if (s.typ[i] == occupied) insert(s.elt[i]);                           \
}                                                                             \
                                                                              \
HashSet& HashSet::operator = (HashSet const& s) {                             \
    /* Make sure we handle "x = x" correctly. */                              \
    if (this == &s) return *this;                                             \
                                                                              \
    delete [] elt;                                                            \
    delete [] typ;                                                            \
    init(s.size());                                                           \
                                                                              \
    for (int i = 0; i < s.tsize; i++)                                         \
        if (s.typ[i] == occupied) insert(s.elt[i]);                           \
                                                                              \
    return *this;                                                             \
}                                                                             \
                                                                              \
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
const int HashSet::prime_table[13] = {                                        \
    3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43                           \
};                                                                            \
                                                                              \
void HashSet::resize() {                                                      \
    /*                                                                        \
     * Save away old contents                                                 \
     */                                                                       \
    Typ* old_typ = typ;                                                       \
    Elt* old_elt = elt;                                                       \
    int  old_size = tsize;                                                    \
                                                                              \
    init(count);                                                              \
                                                                              \
    /*                                                                        \
     * Restore old contents                                                   \
     */                                                                       \
    for (int i = 0; i < old_size; i++) {                                      \
        if (old_typ[i] == occupied) {                                         \
            insert(old_elt[i]);                                               \
        }                                                                     \
    }                                                                         \
                                                                              \
    /*                                                                        \
     * Free storage                                                           \
     */                                                                       \
    delete [] old_elt;                                                        \
    delete [] old_typ;                                                        \
}                                                                             \
                                                                              \
int HashSet::find_index(Elt k) const {                                        \
    int h1 = hasher(k);                                                       \
    int first = (h1 & HASHSET_PRECMASK) >> (HASHSET_PRECBITS - slotBits);     \
                                                                              \
    /*                                                                        \
     * First try without computing secondary hash function                    \
     */                                                                       \
    Typ t = typ[first];                                                       \
    if ((t == empty) || ((t == occupied) && (comparer(elt[first],k)))) {      \
        return first;                                                         \
    }                                                                         \
                                                                              \
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
        if ((t == empty) || ((t == occupied) && (comparer(elt[i],k)))) {      \
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
void HashSet::check() {                                                       \
    int i, j;                                                                 \
                                                                              \
    /* Check for duplicates */                                                \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] != occupied) continue;                                     \
                                                                              \
        for (j = i+1; j < tsize; j++) {                                       \
            if (typ[j] == occupied) {                                         \
                assert(! comparer(elt[i], elt[j]));                           \
            }                                                                 \
        }                                                                     \
    }                                                                         \
                                                                              \
    /* Check that hash values for all available elts do not wrap */           \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] != occupied) continue;                                     \
                                                                              \
        bool* marked = new bool[tsize];                                       \
        for (j = 0; j < tsize; j++) {                                         \
            marked[j] = FALSE;                                                \
        }                                                                     \
                                                                              \
        int h1 = hasher(elt[i]);                                              \
        int h2 = hash2(h1);                                                   \
        int first = (h1 & HASHSET_PRECMASK) >> (HASHSET_PRECBITS - slotBits); \
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
    /* Check that there are no empty slots in hash sequence for each elt */   \
    for (i = 0; i < tsize; i++) {                                             \
        if (typ[i] != occupied) continue;                                     \
                                                                              \
        int h1 = hasher(elt[i]);                                              \
        int h2 = hash2(h1);                                                   \
        int first = (h1 & HASHSET_PRECMASK) >> (HASHSET_PRECBITS - slotBits); \
                                                                              \
        /* Loop at most tsize times */                                        \
        for (j = 0; j < tsize; j++) {                                         \
            int index = (first + h2*j) & mask;                                \
                                                                              \
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
void HashSet##_Elements::skip_empty() {                                       \
    while ((index < table->tsize) &&                                          \
           (table->typ[index] != HashSet::occupied)) {                        \
        index++;                                                              \
    }                                                                         \
}                                                                             \

#endif /* _OHASHSET_H */
