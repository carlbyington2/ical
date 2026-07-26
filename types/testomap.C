/* Copyright (c) 1995 by Sanjay Ghemawat */
/*
 * Test open hash map.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ohashmap.h"
#include "hashfuncs.h"

declareOpenHashMap(IMap,int,char const*,hash_int,cmp_int)
implementOpenHashMap(IMap,int,char const*,hash_int,cmp_int)

//#define LOG_ASSERTS

#ifdef LOG_ASSERTS
#define ASSERT(x) do {fprintf(stderr, "ASSERT: %s\n", #x);assert(x);} while (0)
#else
#define ASSERT(x) assert(x)
#endif

/*
 * Check if M2 is a subset of M1.
 */
static int subset(IMap& m1, IMap& m2) {
    for (IMap_Bindings i = &m1; i.ok(); i.next()) {
        if (! m2.contains(i.key())) {
            return 0;
        }
        if (m2.fetch(i.key()) != i.val()) {
            return 0;
        }
    }
    return 1;
}

/*
 * Compare two maps.
 */
static int compare(IMap& m1, IMap& m2) {
    return (subset(m1, m2) && subset(m2, m1));
}

/*
 * Copy map contents into another map.
 */
static void copy(IMap& m1, IMap& m2) {
    /* Clear destination */
    m2.clear();

    /* Copy into destination */
    for (IMap_Bindings iter = &m1; iter.ok(); iter.next()) {
        m2.store(iter.key(), iter.val());
    }
}

/*
 * Get map size by using iterators.
 */
static int num_iterations(const IMap& m) {
    int count = 0;
    for (IMap_Bindings i = &m; i.ok(); i.next()) {
        count++;
    }
    return count;
}

/*
 * Check that iteration over map yields specified key,value pair
 */
static int iteration_contains(const IMap& m, int key, const char* value) {
    for (IMap_Bindings i = &m; i.ok(); i.next()) {
        if (i.key() == key) {
            ASSERT(i.val() == value);
            return 1;
        }
    }
    return 0;
}

/* Check deletion from iterator */
static void check_del_iter(const IMap& m) {
    for (int n = 1; n <= m.size(); n++) {
        // Delete the first "n" elements of set via iterator
        IMap copy = m;
        int c = 0;
        for (IMap_Bindings i = &copy; i.ok(); i.next()) {
            c++;
            if (c <= n) i.del();
        }
        ASSERT(copy.size() == (m.size() - n));

        // Delete the last "n" elements of set via iterator
        copy = m;
        c = m.size();
        for (IMap_Bindings j = &copy; j.ok(); j.next()) {
            if (c <= n) j.del();
            c--;
        }
        ASSERT(copy.size() == (m.size() - n));
    }
}

/*
 * Some common strings.
 */
static const char* one = "one";
static const char* two = "two";
static const char* three = "three";
static const char* four = "four";
static const char* five = "five";

static void black_empty();
static void black_single();
static void black_multiple();

static void black_box() {
    /*
     * Testing strategy -
     *
     * - Operations on empty maps.
     * - Operations on singleton maps.
     * - Operations on larger maps.
     */

    black_empty();
    black_single();
    black_multiple();
}

static void black_empty() {
    /* Empty map tests. */

    int i;
    const char* value;

    IMap empty;
    empty.check();

    /* Check size */
    ASSERT(empty.size() == 0);

    /* Check contains */
    for (i = -3; i <= 3; i++) {
        ASSERT(! empty.contains(i));
    }

    /* Cannot check unary fetch because of requires clause */

    /* Check fetch */
    for (i = -5; i <= 5; i++) {
        ASSERT(! empty.fetch(i, value));
    }

    /* Check iterator */
    {
        check_del_iter(empty);
        ASSERT(num_iterations(empty) == 0);

        IMap_Bindings iter = &empty;
        ASSERT(! iter.ok());
    }

    /* Check store */
    {
        IMap single;
        single.check();
        single.store(1, one);
        single.check();

        ASSERT(single.size() == 1);
        ASSERT(single.contains(1));
        ASSERT(single.fetch(1, value));
        ASSERT(value == one);
    }

    /* Check insert */
    {
        IMap single;
        single.check();
        single.store(1, one);
        single.check();
        
        ASSERT(single.size() == 1);
        ASSERT(single.contains(1));
        ASSERT(single.fetch(1, value));
        ASSERT(value == one);
    }

    /* Check remove */
    {
        IMap empty2;
        empty2.check();

        ASSERT(empty2.size() == 0);
        empty2.remove(1);
        empty2.check();
        ASSERT(empty2.size() == 0);
    }
}

static void black_single() {
    /* Single element map tests */

    int i;
    const char* value;

    IMap single;
    single.check();
    single.store(2, two);
    single.check();

    ASSERT(single.size() == 1);

    /* Check contains */
    for (i = -3; i <= 3; i++) {
        ASSERT(single.contains(i) == (i == 2));
    }

    /* Check fetch */
    ASSERT(single.fetch(2) == two);

    /* Check fetch */
    for (i = -5; i <= 5; i++) {
        ASSERT(single.fetch(i, value) == ((i == 2) && (value == two)));
    }

    /* Check iterator */
    {
        check_del_iter(single);
        ASSERT(num_iterations(single) == 1);
        ASSERT(iteration_contains(single, 2, two));
    }

    /* Check store */
    {
        IMap temp;
        temp.check();
        copy(single, temp);
        temp.check();

        ASSERT(temp.size() == 1);
        temp.store(2, three);
        temp.check();
        ASSERT(temp.size() == 1);
        ASSERT(temp.contains(2));
        ASSERT(temp.fetch(2) == three);

        copy(single, temp);
        ASSERT(temp.size() == 1);
        temp.store(3, three);
        temp.check();
        ASSERT(temp.size() == 2);
        ASSERT(temp.contains(2));
        ASSERT(temp.contains(3));
        ASSERT(temp.fetch(2) == two);
        ASSERT(temp.fetch(3) == three);

        ASSERT(num_iterations(temp) == 2);
        ASSERT(iteration_contains(temp, 2, two));
        ASSERT(iteration_contains(temp, 3, three));
    }

    /* Check insert */
    {
        IMap temp;
        temp.check();
        copy(single, temp);
        temp.check();

        ASSERT(temp.size() == 1);
        temp.store(3, three);
        temp.check();
        ASSERT(temp.size() == 2);
        ASSERT(temp.contains(2));
        ASSERT(temp.contains(3));
        ASSERT(temp.fetch(2) == two);
        ASSERT(temp.fetch(3) == three);

        ASSERT(num_iterations(temp) == 2);
        ASSERT(iteration_contains(temp, 2, two));
        ASSERT(iteration_contains(temp, 3, three));
    }

    /* Check remove */
    {
        IMap temp;
        temp.check();
        copy(single, temp);
        temp.check();

        temp.remove(5);
        temp.check();
        ASSERT(compare(temp, single));

        temp.remove(2);
        temp.check();
        ASSERT(temp.size() == 0);
        ASSERT(! temp.contains(2));
    }
}

static void black_multiple() {
    int i;
    const char* value;
    IMap multi3, multi4, multi5;

    multi3.check();
    multi3.store(1, one);
    multi3.check();
    multi3.store(2, two);
    multi3.check();
    multi3.store(3, three);
    multi3.check();

    multi4.check();
    multi4.store(1, one);
    multi4.check();
    multi4.store(2, two);
    multi4.check();
    multi4.store(3, three);
    multi4.check();
    multi4.store(4, four);
    multi4.check();

    multi5.check();
    multi5.store(1, one);
    multi5.check();
    multi5.store(2, two);
    multi5.check();
    multi5.store(3, three);
    multi5.check();
    multi5.store(4, four);
    multi5.check();
    multi5.store(5, five);
    multi5.check();

    /* Check size */
    ASSERT(multi3.size() == 3);
    ASSERT(multi4.size() == 4);
    ASSERT(multi5.size() == 5);

    /* Check contains. */
    ASSERT(multi3.contains(1));
    ASSERT(multi3.contains(2));
    ASSERT(multi3.contains(3));

    ASSERT(multi4.contains(1));
    ASSERT(multi4.contains(2));
    ASSERT(multi4.contains(3));
    ASSERT(multi4.contains(4));

    ASSERT(multi5.contains(1));
    ASSERT(multi5.contains(2));
    ASSERT(multi5.contains(3));
    ASSERT(multi5.contains(3));
    ASSERT(multi5.contains(5));

    /* Check fetch */
    ASSERT(multi3.fetch(1) == one);
    ASSERT(multi3.fetch(2) == two);
    ASSERT(multi3.fetch(3) == three);

    ASSERT(multi4.fetch(1) == one);
    ASSERT(multi4.fetch(2) == two);
    ASSERT(multi4.fetch(3) == three);
    ASSERT(multi4.fetch(4) == four);

    ASSERT(multi5.fetch(1) == one);
    ASSERT(multi5.fetch(2) == two);
    ASSERT(multi5.fetch(3) == three);
    ASSERT(multi5.fetch(4) == four);
    ASSERT(multi5.fetch(5) == five);

    /* Check fetch */
    for (i = -5; i <= 0; i++) {
        ASSERT(! multi3.fetch(i, value));
        ASSERT(! multi4.fetch(i, value));
        ASSERT(! multi5.fetch(i, value));
    }
    ASSERT(multi3.fetch(1, value) && (value == one));
    ASSERT(multi3.fetch(2, value) && (value == two));
    ASSERT(multi3.fetch(3, value) && (value == three));
    ASSERT(! multi3.fetch(4, value));
    ASSERT(! multi3.fetch(5, value));

    ASSERT(multi4.fetch(1, value) && (value == one));
    ASSERT(multi4.fetch(2, value) && (value == two));
    ASSERT(multi4.fetch(3, value) && (value == three));
    ASSERT(multi4.fetch(4, value) && (value == four));
    ASSERT(! multi4.fetch(5, value));

    ASSERT(multi5.fetch(1, value) && (value == one));
    ASSERT(multi5.fetch(2, value) && (value == two));
    ASSERT(multi5.fetch(3, value) && (value == three));
    ASSERT(multi5.fetch(4, value) && (value == four));
    ASSERT(multi5.fetch(5, value) && (value == five));

    /* Check iterator */
    {
        check_del_iter(multi3);
        check_del_iter(multi4);
        check_del_iter(multi5);

        ASSERT(num_iterations(multi3) == 3);
        ASSERT(iteration_contains(multi3, 1, one));
        ASSERT(iteration_contains(multi3, 2, two));
        ASSERT(iteration_contains(multi3, 3, three));

        ASSERT(num_iterations(multi4) == 4);
        ASSERT(iteration_contains(multi4, 1, one));
        ASSERT(iteration_contains(multi4, 2, two));
        ASSERT(iteration_contains(multi4, 3, three));
        ASSERT(iteration_contains(multi4, 4, four));

        ASSERT(num_iterations(multi5) == 5);
        ASSERT(iteration_contains(multi5, 1, one));
        ASSERT(iteration_contains(multi5, 2, two));
        ASSERT(iteration_contains(multi5, 3, three));
        ASSERT(iteration_contains(multi5, 4, four));
        ASSERT(iteration_contains(multi5, 5, five));
    }

    /* Check store */
    {
        IMap temp;
        temp.check();
        copy(multi3, temp);
        temp.check();

        ASSERT(compare(multi3, temp));

        /* Store existing element */
        temp.store(2, five);
        temp.check();
        ASSERT(temp.size() == multi3.size());
        ASSERT(temp.contains(2));
        ASSERT(temp.fetch(2) == five);
        temp.remove(2);
        temp.check();
        temp.store(2, multi3.fetch(2));
        temp.check();
        ASSERT(compare(multi3, temp));

        /* Store non-existent element */
        copy(multi4, temp);
        temp.check();
        ASSERT(compare(multi4, temp));
        temp.store(5, five);
        temp.check();
        ASSERT(compare(multi5, temp));
        temp.remove(5);
        temp.check();
        ASSERT(compare(multi4, temp));
    }

    /* Check insert */
    {
        IMap temp;
        temp.check();
        copy(multi4, temp);
        temp.check();

        ASSERT(compare(multi4, temp));
        ASSERT(temp.size() == 4);
        temp.store(5, five);
        temp.check();
        ASSERT(compare(multi5, temp));

        copy(multi3, temp);
        temp.store(4, four);
        temp.check();
        temp.store(5, five);
        temp.check();
        ASSERT(compare(multi5, temp));
    }

    /* Check remove */
    {
        IMap temp, empty;

        /* Check removal of existing elements */
        temp.check();
        copy(multi3, temp);
        temp.check();
        ASSERT(compare(multi3, temp));
        temp.remove(1);
        temp.check();
        temp.remove(2);
        temp.check();
        temp.remove(3);
        temp.check();
        ASSERT(compare(empty, temp));

        copy(multi3, temp);
        temp.check();
        ASSERT(compare(multi3, temp));
        temp.remove(3);
        temp.check();
        temp.remove(2);
        temp.check();
        temp.remove(1);
        temp.check();
        ASSERT(compare(empty, temp));

        copy(multi5, temp);
        temp.check();
        ASSERT(compare(multi5, temp));
        temp.remove(5);
        temp.check();
        ASSERT(compare(multi4, temp));
        temp.remove(4);
        temp.check();
        ASSERT(compare(multi3, temp));
        temp.remove(1);
        temp.check();
        temp.remove(2);
        temp.check();
        temp.remove(3);
        temp.check();
        ASSERT(compare(empty, temp));

        /* Check removal of non-existent elements */
        copy(multi4, temp);
        temp.check();
        for (i = -5; i <= 0; i++) {
            temp.remove(i);
            temp.check();
            ASSERT(compare(multi4, temp));
        }
        for (i = 5; i <= 10; i++) {
            temp.remove(i);
            temp.check();
            ASSERT(compare(multi4, temp));
        }
    }

    /* Check large number of entries */
    {
        IMap map;

        map.check();
        for (i = 0; i < 1000; i++) {
            char* val = new char[20];
            sprintf(val, "%d", i);
            map.store(i, val);
            ASSERT(num_iterations(map) == i+1);
        }
        map.check();

        char* copy = new char[20];
        for (i = 0; i < 1000; i++) {
            char const* val = map.fetch(i);
            sprintf(copy, "%d", i);
            ASSERT(strcmp(copy, val) == 0);
        }
        delete copy;

        for (i = 0; i < 1000; i++) {
            delete [] (char*)map.fetch(i);
            map.remove(i);
            ASSERT(num_iterations(map) == (999-i));
        }
        map.check();
    }

    /* Check prediction */
    {
        IMap map(1000);

        map.check();
        for (i = 0; i < 1000; i++) {
            char* val = new char[20];
            sprintf(val, "%d", i);
            map.store(i, val);
            ASSERT(num_iterations(map) == i+1);
        }
        map.check();

        char* copy = new char[20];
        for (i = 0; i < 1000; i++) {
            char const* val = map.fetch(i);
            sprintf(copy, "%d", i);
            ASSERT(strcmp(copy, val) == 0);
        }
        delete copy;

        for (i = 0; i < 1000; i++) {
            delete [] (char*)map.fetch(i);
            map.remove(i);
            ASSERT(num_iterations(map) == (999-i));
        }
        map.check();
    }
}

/*
 * Glass box tests.
 */
static void glass_box() {
    // ...
}

int
main() {
    black_box();
    glass_box();
    return 0;
}
