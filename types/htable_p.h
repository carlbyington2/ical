/* Copyright (c) 1995 by Sanjay Ghemawat */
// This file is designed to be included multiple times and therefore
// no header guards should be placed on it.

// "Template" for generating various kinds of hash tables.
//
// See "htable.h" for macros that should be defined before including this.

// Some static data/functions
#ifndef HTABLE_STATICS
#define HTABLE_STATICS

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

inline unsigned int htable_hash_elem(HKEY x) {
    // The multiplier used below is "int(PHI * 2^k)" where PHI denotes
    // the golden ratio and "k" is 31.  This multiplier value is
    // suggested as a good one by Knuth.
    //
    // The resulting hash value is converted into the range
    // "[0..2^p-1]" by extracting the top "p" bits of the bottom k bits
    // of the hash value.  An easy way to do this is to right shift the
    // result by "k - p".
    return (((unsigned int) HASHER(x)) * 3474701532U) & 0x7FFFFFFFU;
}

inline unsigned int htable_hash(unsigned int h, unsigned int slot_bits) {
    return h >> (31U - slot_bits);
}

static const int htable_default_max_occupancy = 50;
// Default maximum occupancy of hash table in percent.
// If the table becomes more than this full, it is enlarged.

static const int htable_min_occupancy_divisor = 4;
// Divisor to obtain minimum occupancy from maximum occupancy.
// We set this to four so that the table does not keep bouncing
// back and forth between two contiguous sizes as a small number
// of elements are inserted and removed.

#endif /* HTABLE_STATICS */

HTABLE::HTABLE() {
    max_occupancy = htable_default_max_occupancy;
    init(1);
}

HTABLE::~HTABLE() {
    delete table;
}

HTABLE::HTABLE(HTABLE const& x) {
    max_occupancy = htable_default_max_occupancy;
    init(1);
    *this = x;
}

HTABLE& HTABLE::operator = (HTABLE const& x) {
    // This check is necessary to deal with aliasing between this and x.
    if (&x != this) {
        // Remove current contents
        clear();

        // Resize set to right size in one shot
        resize(x.size());

        // Populate with elements from x
        for (int i = x.tsize-1; i >= 0; i--) {
            if (x.table->is_full(i)) {
                HKEY k = x.table->key(i);
#ifdef HVAL
                unsafe_insert(k, x.table->val(i), htable_hash_elem(k));
#else
                unsafe_insert(k, htable_hash_elem(k));
#endif
            }
        }
    }
    return *this;
}

unsigned int HTABLE::find_index(HKEY key) const {
#if 0
    static int xcount;
    if (xcount == 0) {
        xcount = 1000000;
        fprintf(stderr, "tsize = %8u, count = %8u, delcount = %8u\n",
                tsize, count, delcount);
        report_stats("periodic");
    }
    xcount--;
#endif

    HTABLEREP* tbl = table;
    unsigned int h = htable_hash_elem(key);
    unsigned int index = htable_hash(h, slot_bits);
    unsigned int msk = mask;
    
    while (1) {
        if (tbl->is_empty(index)) {
            return index;
        } else if (tbl->is_del(index)) {
            // This is a suitable candidate for insertion, but we need to
            // check that the element is not in the table.
            unsigned int del_index = index;
            while (1) {
                index = (index + 1) & msk;
                if (tbl->is_empty(index)) {
                    return del_index;
                } else if (tbl->is_full(index) && HEQUAL(tbl->key(index),key)){
                    return index;
                }
            }
        } else if (HEQUAL(tbl->key(index), key)) {
            return index;
        }
        index = (index + 1) & msk;
    }
}

int HTABLE::contains(HKEY x) const {
    unsigned int index = find_index(x);
    return (table->is_full(index));
}

#ifdef HVAL
int HTABLE::find(HKEY x, HVAL& result) const {
    unsigned int index = find_index(x);
    if (table->is_full(index)) {
        result = table->val(index);
        return 1;
    } else {
        return 0;
    }
}
#endif

#ifdef HVAL
int HTABLE::insert(HKEY key, HVAL val)
#else
int HTABLE::insert(HKEY key)
#endif
{
    HTABLEREP* tbl = table;
    unsigned int index = find_index(key);

#ifdef HVAL
    tbl->val(index) = val;
#endif

    if (tbl->is_full(index)) {
        return 0;
    } else {
        int is_new = tbl->is_empty(index);
        count++;
        tbl->store(index, key);
        if (is_new) {
            assert((count + delcount) <= tsize);
            check_enlarge();
        } else {
            delcount--;
        }
        return 1;
    }
}

#ifdef HVAL
void HTABLE::unsafe_insert(HKEY key, HVAL val, unsigned int h)
#else
void HTABLE::unsafe_insert(HKEY key, unsigned int h)
#endif
{
    assert(delcount == 0);

    HTABLEREP* tbl = table;
    unsigned int index = htable_hash(h, slot_bits);
    if (tbl->is_full(index)) {
        // Need secondary probes
        unsigned int msk = mask;
        do {
            index = (index + 1) & msk;
        } while (tbl->is_full(index));
    }

    count++;
    tbl->store(index, key);
#ifdef HVAL
    tbl->val(index) = val;
#endif
}

int HTABLE::remove(HKEY key) {
    unsigned int index = find_index(key);
    if (table->is_full(index)) {
        table->kill(index);
        count--;
        delcount++;

        // XXX Automatically shrink?
        // if (count <= shrink_size) resize(count);

        return 1;
    } else {
        return 0;
    }
}

void HTABLE::clear() {
    HTABLEREP* tbl = table;
    for (int i = tsize-1; i >= 0; i--)
        tbl->clear(i);
    count = 0;
    delcount = 0;
}

void HTABLE::set_occupancy(int x) {
    // Do not go outside range 10..90%
    if (x < 10) x = 10;
    if (x > 90) x = 90;

    if (x != max_occupancy) {
        max_occupancy = x;
        resize(count);
    }
}

unsigned int HTABLE::table_size(unsigned int n) const {
    // Search for the correct power of 2.
    unsigned int power = 1;
    while ((power <= n) || ((power*((double)max_occupancy)) <= ((n+1)*100.0)))
        power <<= 1;
    return power;
}

void HTABLE::check_enlarge() {
    if ((count >= enlarge_size) || ((count + delcount) >= tsize)) {
        // We could optionally just "resize(count)", but the following
        // call helps us deal better with concurrent insertions/deletions.
        //resize(count + delcount);
        resize(count);
    }
}

void HTABLE::init(int n) {
    count = 0;
    delcount = 0;
    tsize = table_size(n);
    mask = tsize-1;

    // Compute "lg(tsize)"
    slot_bits = 0;
    unsigned int x = tsize;
    while (x > 1) {
        x >>= 1;
        slot_bits++;
    }

    // Compute resize thresholds
    enlarge_size = (unsigned int) (((double)tsize) * (((double)max_occupancy)
                                                      / 100.0));
    if (enlarge_size < 1) enlarge_size = 1;
    if (enlarge_size > tsize) enlarge_size = tsize;
    shrink_size = enlarge_size / htable_min_occupancy_divisor;
    if (shrink_size < 0) shrink_size = 0;
    assert(shrink_size < tsize);

    table = new HTABLEREP(tsize);
}

void HTABLE::resize(int n) {
    if (n < count) n = count;

    // Save old contents and make new correctly sized table
    HTABLEREP* old = table;
    unsigned int old_size = tsize;
    init(n);
    assert(tsize > n);

    // Now restore the old contents
    for (int i = old_size-1; i >= 0; i--) {
        if (old->is_full(i)) {
            HKEY k = old->key(i);
#ifdef HVAL
            unsafe_insert(k, old->val(i), htable_hash_elem(k));
#else
            unsafe_insert(k, htable_hash_elem(k));
#endif
        }
    }

    delete old;
}

void HTABLE::check() const {
    int i, j;

    /* Check for duplicates */
    for (i = 0; i < tsize; i++) {
        if (!table->is_full(i)) continue;
        for (j = i+1; j < tsize; j++) {
            if (!table->is_full(j)) continue;
            assert(! HEQUAL(table->key(i), table->key(j)));
        }
    }

    /* Check that hash values for all available elts do not wrap */
    int* marked = new int[tsize];
    for (i = 0; i < tsize; i++) {
        if (!table->is_full(i)) continue;

        for (j = 0; j < tsize; j++) {
            marked[j] = 0;
        }

        unsigned int h = htable_hash_elem(table->key(i));
        unsigned int h1 = htable_hash(h, slot_bits);
        unsigned int index = h1;

        for (j = 0; j < tsize; j++) {
            assert(! marked[index]);
            marked[index] = 1;
            index = (index + 1) & mask;
        }
    }
    delete [] marked;

    /* Check that there are no empty slots in hash sequence for each elt */
    for (i = 0; i < tsize; i++) {
        if (!table->is_full(i)) continue;

        unsigned int h = htable_hash_elem(table->key(i));
        unsigned int h1 = htable_hash(h, slot_bits);
        unsigned int index = h1;

        /* Loop at most tsize times */
        for (j = 0; j < tsize; j++) {
            // Check for hole
            assert(! table->is_empty(index));

            if (index == i) break;
            index = (index + 1) & mask;
        }
    }

    /* count */
    j = 0;
    for (i = 0; i < tsize; i++) {
        if (table->is_full(i)) j++;
    }
    assert(count == j);

    /* delcount */
    j = 0;
    for (i = 0; i < tsize; i++) {
        if (table->is_del(i)) j++;
    }
    assert(delcount == j);

    /* tsize is power of two */
    i = 1;
    while (i < tsize) {
        i = i << 1;
    }
    assert(i == tsize);

    /* Check that at least one slot is empty */
    assert(tsize > (count + delcount));

    /* mask */
    assert(mask == (tsize - 1));
}

void HTABLE::report_stats(char const* msg) const {
    if (count == 0) return;

    // Count number of extra probes to get to each element
    int probes = 0;
    int max_probes = 0;
    for (int index = tsize-1; index >= 0; index--) {
        if (!table->is_full(index)) continue;
        unsigned int h = htable_hash_elem(table->key(index));
        unsigned int j = htable_hash(h, slot_bits);
        unsigned int p = 0;
        while (j != index) {
            j = (j + 1) & mask;
            p++;
        }
        if (p > max_probes) {
            max_probes = p;
        }
        probes += p;
    }

    fprintf(stderr,
            "%s: size %6d, present %6d, deleted %6d, "
            "probes(avg/max)= %5.3f/%d\n",
            msg, tsize, count, delcount,
            ((double) (probes+count)) / ((double) count),
            max_probes);
}

void HTABLE::Elements::del() {
    HTABLE* t = ((HTABLE*) table);

    assert(index >= 0);
    assert(t->table->is_full(index));

    t->count--;
    t->delcount++;
    t->table->kill(index);
}
