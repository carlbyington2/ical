/* Copyright (c) 1995 by Sanjay Ghemawat */
/* Time open hash table. */

//#define MEASURE_FIXED
//#define MEASURE_ARRAY
//#define MEASURE_OLD_SET
//#define MEASURE_OLD_MAP
#define MEASURE_SET
#define MEASURE_MAP
//#define MEASURE_SPARSE_SET
//#undef  MEASURE_STRING_SET

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Measure resource usage.
 */

extern "C" {
#include <sys/time.h>
#include <sys/resource.h>
}

class Rusage {
  public:
    /* Start collecting usage */
    Rusage();

    /* Reset collection */
    void Reset();

    /* Show usage */
    double UserTime();
    double SystemTime();
  private:
    struct rusage start;
};

inline Rusage::Rusage() {
    getrusage(RUSAGE_SELF, &start);
}

inline void Rusage::Reset() {
    getrusage(RUSAGE_SELF, &start);
}

inline double Rusage::UserTime() {
    struct rusage u;

    getrusage(RUSAGE_SELF, &u);

    struct timeval result;
    result.tv_sec  = u.ru_utime.tv_sec  - start.ru_utime.tv_sec;
    result.tv_usec = u.ru_utime.tv_usec - start.ru_utime.tv_usec;

    return double(result.tv_sec) + double(result.tv_usec) / 1000000.0;
}

inline double Rusage::SystemTime() {
    struct rusage u;

    getrusage(RUSAGE_SELF, &u);

    struct timeval result;
    result.tv_sec  = u.ru_stime.tv_sec  - start.ru_stime.tv_sec;
    result.tv_usec = u.ru_stime.tv_usec - start.ru_stime.tv_usec;

    return double(result.tv_sec) + double(result.tv_usec) / 1000000.0;
}

static void measure_fixed();
static void measure_array();
static void measure_old_set();
static void measure_old_map();
static void measure_set();
static void measure_sparse_set();
static void measure_map();
static void measure_string_set();

static const int count = 100000;
#define COUNT_TIMES(var) for (var = count-1; var >= 0; var--)

static void report(char const* title, double t) {
    printf("%-20s %8.2f usec\n",
           title,
           ((double) (t * 1000000.0 / count)));
}

// Generate stamp for this run
static void stamp_run() {
    printf("======\n");
    fflush(stdout);
    system("uname -a");
    system("date");
}

int
main() {
    stamp_run();
    measure_fixed();
    measure_array();
    measure_set();
    measure_sparse_set();
    measure_old_set();
    measure_string_set();
    measure_map();
    measure_old_map();
    return 0;
}

#ifndef MEASURE_FIXED
static void measure_fixed() {}
#else
static void time_fixed_store() {
    char* map = new char[count];
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        map[i] = 0;
    }
    double ut = t.UserTime();

    report("fixed_store", ut);

    delete [] map;
}

static void time_fixed_fetch() {
    char* map = new char[count];
    Rusage t;
    char r;
    int i;

    for (i = 0; i < count; i++) {
        map[i] = 0;
    }

    r = 1;
    t.Reset();
    for (i = 0; i < count; i++) {
        r ^= map[i];
    }
    double ut = t.UserTime();

    report("fixed_fetch", ut);

    delete [] map;
}

static void measure_fixed() {
    time_fixed_store();
    time_fixed_fetch();
}
#endif

#ifndef MEASURE_ARRAY
static void measure_array() {}
#else
#include "Array.h"

declareArray(boolArray,char)
implementArray(boolArray,char)

static void time_array_grow() {
    boolArray array;
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        array.append(0);
    }
    double ut = t.UserTime();

    report("array_grow", ut);
}

static void time_array_grow_predicted() {
    boolArray array(count);
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        array.append(0);
    }
    double ut = t.UserTime();

    report("array_predict/grow", ut);
}

static void time_array_replace() {
    boolArray array(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        array.append(0);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        array[i] = 1;
    }
    double ut = t.UserTime();

    report("array_replace", ut);
}

static void time_array_fetch() {
    boolArray array(count);
    Rusage t;
    char r;
    int i;

    for (i = 0; i < count; i++) {
        array.append(0);
    }

    r = 1;
    t.Reset();
    for (i = 0; i < count; i++) {
        r ^= array[i];
    }
    double ut = t.UserTime();

    report("array_fetch", ut);
}

static void time_array_remove() {
    boolArray array(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        array.append(0);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        array.remove();
    }
    double ut = t.UserTime();

    report("array_remove", ut);
}

static void time_array_store() {
    boolArray array(count);
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        array.append(0);
    }
    double ut = t.UserTime();

    report("store/array", ut);
}

static void measure_array() {
    time_array_grow();
    time_array_grow_predicted();
    time_array_replace();
    time_array_fetch();
    time_array_remove();
}
#endif

#if defined(MEASURE_OLD_SET) || defined(MEASURE_OLD_MAP)
#include "hashfuncs.h"
#include "hashfuncs.C"
#endif

#ifndef MEASURE_OLD_MAP
static void measure_old_map() {}
#else

#include "ohashmap.h"
declareOpenHashMap(OldIntMap,int,char const*,hash_int,cmp_int)
implementOpenHashMap(OldIntMap,int,char const*,hash_int,cmp_int)

static void time_old_map_grow() {
    OldIntMap map;
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        map.store(i, 0);
    }
    double ut = t.UserTime();

    report("oldmap_grow", ut);
}

static void time_old_map_grow_predicted() {
    OldIntMap map(count);
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        map.store(i, 0);
    }
    double ut = t.UserTime();

    report("oldmap_predict/grow", ut);
}

static void time_old_map_replace() {
    OldIntMap map(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        map.store(i, 0);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        map.store(i, 0);
    }
    double ut = t.UserTime();

    report("oldmap_replace", ut);
}

static void time_old_map_fetch() {
    OldIntMap map(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        map.store(i, 0);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        char const* val;
        map.fetch(i, val);
    }
    double ut = t.UserTime();

    report("oldmap_fetch", ut);
}

static void time_old_map_remove() {
    OldIntMap map(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        map.store(i, 0);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        map.remove(i);
    }
    double ut = t.UserTime();

    report("oldmap_remove", ut);
}

static void measure_old_map() {
    time_old_map_grow();
    time_old_map_grow_predicted();
    time_old_map_replace();
    time_old_map_fetch();
    time_old_map_remove();
}
#endif

#ifndef MEASURE_OLD_SET
static void measure_old_set() {}
#else

#include "ohashset.h"
declareOpenHashSet(OldIntSet,int,hash_int,cmp_int)
implementOpenHashSet(OldIntSet,int,hash_int,cmp_int)

static void time_old_set_grow() {
    OldIntSet set;
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("oldset_grow", ut);
}

static void time_old_set_grow_predicted() {
    OldIntSet set(count);
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("oldset_predict/grow", ut);
}

static void time_old_set_replace() {
    OldIntSet set(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(i);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("oldset_replace", ut);
}

static void time_old_set_fetch() {
    OldIntSet set(count);
    Rusage t;
    char r;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(i);
    }

    r = 1;
    t.Reset();
    for (i = 0; i < count; i++) {
        r ^= set.contains(i);
    }
    double ut = t.UserTime();

    report("oldset_fetch", ut);
}

static void time_old_set_remove() {
    OldIntSet set(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(i);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.remove(i);
    }
    double ut = t.UserTime();

    report("oldset_remove", ut);
}

static void measure_old_set() {
    time_old_set_grow();
    time_old_set_grow_predicted();
    time_old_set_replace();
    time_old_set_fetch();
    time_old_set_remove();
}
#endif

#if defined(MEASURE_SET) || defined(MEASURE_SPARSE_SET)
#include "intset.h"
#endif

#ifndef MEASURE_SET
static void measure_set() {}
#else

static void time_set_grow() {
    IntSet set;
    Rusage t;
    int i;

    t.Reset();
    COUNT_TIMES(i) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("set_grow", ut);
}

static void time_set_grow_predicted() {
    IntSet set;
    set.predict(count);
    int i;

    Rusage t;

    t.Reset();
    COUNT_TIMES(i) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("set_predict/grow", ut);
}

static void time_set_replace() {
    IntSet set;
    set.predict(count);
    Rusage t;
    int i;

    COUNT_TIMES(i) {
        set.insert(i);
    }

    t.Reset();
    COUNT_TIMES(i) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("set_replace", ut);
}

static void time_set_fetch() {
    IntSet set;
    set.predict(count);
    Rusage t;
    int r;
    int i;

    COUNT_TIMES(i) {
        set.insert(i);
    }

    r = 1;
    t.Reset();
    COUNT_TIMES(i) {
        r ^= set.contains(i);
    }
    double ut = t.UserTime();

    report("set_fetch", ut);
    //set.report_stats("fetch");
}

static void time_set_remove() {
    IntSet set;
    set.predict(count);
    Rusage t;
    int i;

    COUNT_TIMES(i) {
        set.insert(i);
    }

    t.Reset();
    COUNT_TIMES(i) {
        set.remove(i);
    }
    double ut = t.UserTime();

    report("set_remove", ut);
}

static void measure_set() {
    time_set_grow();
    time_set_grow_predicted();
    time_set_replace();
    time_set_fetch();
    time_set_remove();
}
#endif

#ifndef MEASURE_MAP
static void measure_map() {}
#else

#define HTABLE_IMPLEMENT
#define HTABLE    IntIntMap
#define HKEY      int
#define HVAL      int
#define HASHER(x) (x)
#include "htable.h"

static void time_map_grow() {
    IntIntMap set;
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(i, i);
    }
    double ut = t.UserTime();

    report("map_grow", ut);
}

static void time_map_grow_predicted() {
    IntIntMap set;
    set.predict(count);

    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(i, i);
    }
    double ut = t.UserTime();

    report("map_predict/grow", ut);
}

static void time_map_replace() {
    IntIntMap set;
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(i, i);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.insert(i, i);
    }
    double ut = t.UserTime();

    report("map_replace", ut);
}

static void time_map_fetch() {
    IntIntMap set;
    Rusage t;
    int r;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(i, i);
    }

    r = 1;
    t.Reset();
    for (i = 0; i < count; i++) {
        r ^= set.contains(i);
    }
    double ut = t.UserTime();

    report("map_fetch", ut);
}

static void time_map_remove() {
    IntIntMap set;
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(i, i);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.remove(i);
    }
    double ut = t.UserTime();

    report("map_remove", ut);
}

static void measure_map() {
    time_map_grow();
    time_map_grow_predicted();
    time_map_replace();
    time_map_fetch();
    time_map_remove();
}
#endif

#ifndef MEASURE_SPARSE_SET
static void measure_sparse_set() {}
#else

// Sparse table occupancy
static const int sparse = 50;

static void time_sp_set_grow() {
    IntSet set;
    Rusage t;

    set.set_occupancy(sparse);
    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("sp_set_grow", ut);
}

static void time_sp_set_grow_predicted() {
    IntSet set;
    set.predict(count);

    Rusage t;

    set.set_occupancy(sparse);
    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("sp_set_predict/grow", ut);
}

static void time_sp_set_replace() {
    IntSet set;
    Rusage t;
    int i;

    set.set_occupancy(sparse);
    for (i = 0; i < count; i++) {
        set.insert(i);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.insert(i);
    }
    double ut = t.UserTime();

    report("sp_set_replace", ut);
}

static void time_sp_set_fetch() {
    IntSet set;
    Rusage t;
    int r;
    int i;

    set.set_occupancy(sparse);
    for (i = 0; i < count; i++) {
        set.insert(i);
    }

    r = 1;
    t.Reset();
    for (i = 0; i < count; i++) {
        r ^= set.contains(i);
    }
    double ut = t.UserTime();

    report("sp_set_fetch", ut);
    //set.report_stats("fetch");
}

static void time_sp_set_remove() {
    IntSet set;
    Rusage t;
    int i;

    set.set_occupancy(sparse);
    for (i = 0; i < count; i++) {
        set.insert(i);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.remove(i);
    }
    double ut = t.UserTime();

    report("sp_set_remove", ut);
}

static void measure_sparse_set() {
    time_sp_set_grow();
    time_sp_set_grow_predicted();
    time_sp_set_replace();
    time_sp_set_fetch();
    time_sp_set_remove();
}
#endif

#ifndef MEASURE_STRING_SET
static void measure_string_set() {}
#else

declareOpenHashSet(StringSet,char const*,hash_string,cmp_string)
implementOpenHashSet(StringSet,char const *,hash_string,cmp_string)

static void str_init();
static char const** strings;

static void str_init() {
    /* Initialize strings */
    strings = new const char*[count];

    char* buffer = new char[count * 12];
    for (int i = 0; i < count; i++) {
        sprintf(buffer, "str%d", i);
        strings[i] = buffer;
        buffer += 12;
    }
}

static void time_sset_grow() {
    StringSet set;
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(strings[i]);
    }
    double ut = t.UserTime();

    report("sset_grow", ut);
}

static void time_sset_grow_predicted() {
    StringSet set(count);
    Rusage t;

    t.Reset();
    for (int i = 0; i < count; i++) {
        set.insert(strings[i]);
    }
    double ut = t.UserTime();

    report("sset_predict/grow", ut);
}

static void time_sset_replace() {
    StringSet set(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(strings[i]);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.insert(strings[i]);
    }
    double ut = t.UserTime();

    report("sset_replace", ut);
}

static void time_sset_fetch() {
    StringSet set(count);
    Rusage t;
    char r;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(strings[i]);
    }

    r = 1;
    t.Reset();
    for (i = 0; i < count; i++) {
        r ^= set.contains(strings[i]);
    }
    double ut = t.UserTime();

    report("sset_fetch", ut);
}

static void time_sset_remove() {
    StringSet set(count);
    Rusage t;
    int i;

    for (i = 0; i < count; i++) {
        set.insert(strings[i]);
    }

    t.Reset();
    for (i = 0; i < count; i++) {
        set.remove(strings[i]);
    }
    double ut = t.UserTime();

    report("sset_remove", ut);
}

static void measure_string_set() {
    str_init();
    time_sset_grow();
    time_sset_grow_predicted();
    time_sset_replace();
    time_sset_fetch();
    time_sset_remove();
}
#endif



