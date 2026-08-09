/* Copyright (c) 1995 by Sanjay Ghemawat */
// Tests for "HashTable".

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define HTABLE_IMPLEMENT
#define HTABLE          IntSet
#define HKEY            int
#define HASHER(x)       (x)
#include "htable.h"

typedef IntSet ISet;
typedef IntSet::Elements ISet_Elements;

//#define LOG_ASSERTS

#ifdef LOG_ASSERTS
#define ASSERT(x) do {fprintf(stderr, "ASSERT: %s\n", #x);assert(x);} while (0)
#else
#define ASSERT(x) assert(x)
#endif

#ifdef NDEBUG
static void check(ISet& s) {}
#else
static void check(ISet& s) {s.check();}
#endif

/*
 * Check if M2 is a subset of M1.
 */
static int subset(ISet& m1, ISet& m2) {
    int val;
    ISet_Elements i = &m1;
    while (i.get(val)) {
        if (! m2.contains(val)) return 0;
    }
    return 1;
}

/*
 * Compare two sets.
 */
static int compare(ISet& m1, ISet& m2) {
    return (subset(m1, m2) && subset(m2, m1));
}

/*
 * Copy set contents into another set.
 */
static void copy(ISet& m1, ISet& m2) {
    m2 = m1;
}

/*
 * Get set size by using iterators.
 */
static int num_iterations(const ISet& m) {
    int count = 0;
    int val;
    ISet_Elements i = &m;
    while (i.get(val))
        count++;
    return count;
}

/*
 * Check that iteration over set yields specified key,value pair
 */
static int iteration_contains(const ISet& m, int key) {
    int val;
    ISet_Elements i = &m;
    while (i.get(val)) {
        if (val == key) return 1;
    }
    return 0;
}

static void black_empty();
static void black_single();
static void black_multiple();

static void black_box() {
    /*
     * Testing strategy -
     *
     * - Operations on empty sets.
     * - Operations on singleton sets.
     * - Operations on larger sets.
     */

    black_empty();
    black_single();
    black_multiple();
}

static void black_empty() {
    /* Empty set tests. */

    int i;

    ISet empty;
    check(empty);

    /* Check size */
    ASSERT(empty.size() == 0);

    /* Check contains */
    for (i = -3; i <= 3; i++) {
        ASSERT(! empty.contains(i));
    }

    /* Check iterator */
    {
        ASSERT(num_iterations(empty) == 0);

        ISet_Elements iter = &empty;
        int x;
        ASSERT(! iter.get(x));
    }

    /* Check copy */
    {
        ISet temp = empty;
        check(temp);
        ASSERT(compare(temp, empty));
    }

    /* Check insert */
    {
        ISet single;
        check(single);
        single.insert(1);
        check(single);

        ASSERT(single.size() == 1);
        ASSERT(single.contains(1));
        ASSERT(num_iterations(single) == 1);

        single.insert(1);

        ASSERT(single.size() == 1);
        ASSERT(single.contains(1));
        ASSERT(num_iterations(single) == 1);
    }

    /* Check remove */
    {
        ISet empty2;
        check(empty2);

        ASSERT(empty2.size() == 0);
        empty2.remove(1);
        check(empty2);
        ASSERT(empty2.size() == 0);
    }

    /* Check clear */
    {
        ISet empty2;
        check(empty2);

        ASSERT(empty2.size() == 0);
        empty2.insert(1);
        empty2.clear();
        check(empty2);
        ASSERT(empty2.size() == 0);
        compare(empty2, empty);
    }
}

static void black_single() {
    /* Single element set tests */

    int i;

    ISet single;
    check(single);
    single.insert(2);
    check(single);

    ASSERT(single.size() == 1);

    /* Check contains */
    for (i = -3; i <= 3; i++) {
        ASSERT(single.contains(i) == (i == 2));
    }

    /* Check iterator */
    {
        ASSERT(num_iterations(single) == 1);
        ASSERT(iteration_contains(single, 2));
    }

    /* Check copy */
    {
        ISet temp = single;
        check(temp);
        ASSERT(compare(temp, single));

        temp.remove(2);
        ASSERT(temp.size() == 0);

        ISet temp2 = temp;
        ASSERT(temp2.size() == 0);
    }

    /* Check insert */
    {
        ISet temp;
        check(temp);
        copy(single, temp);
        check(temp);

        ASSERT(temp.size() == 1);
        temp.insert(2);
        check(temp);
        ASSERT(temp.size() == 1);
        ASSERT(temp.contains(2));

        copy(single, temp);
        ASSERT(temp.size() == 1);
        temp.insert(3);
        check(temp);
        ASSERT(temp.size() == 2);
        ASSERT(temp.contains(2));
        ASSERT(temp.contains(3));

        ASSERT(num_iterations(temp) == 2);
        ASSERT(iteration_contains(temp, 2));
        ASSERT(iteration_contains(temp, 3));

        temp.insert(2);
        temp.insert(3);
        ASSERT(num_iterations(temp) == 2);
    }

    /* Check insert */
    {
        ISet temp;
        check(temp);
        copy(single, temp);
        check(temp);

        ASSERT(temp.size() == 1);
        temp.insert(3);
        check(temp);
        ASSERT(temp.size() == 2);
        ASSERT(temp.contains(2));
        ASSERT(temp.contains(3));

        ASSERT(num_iterations(temp) == 2);
        ASSERT(iteration_contains(temp, 2));
        ASSERT(iteration_contains(temp, 3));
    }

    /* Check remove */
    {
        ISet temp;
        check(temp);
        copy(single, temp);
        check(temp);

        temp.remove(5);
        check(temp);
        ASSERT(compare(temp, single));
        ASSERT(num_iterations(temp) == 1);

        temp.remove(2);
        check(temp);
        ASSERT(temp.size() == 0);
        ASSERT(! temp.contains(2));
        ASSERT(num_iterations(temp) == 0);
    }

    /* Check clear */
    {
        ISet temp;
        check(temp);
        copy(single, temp);
        check(temp);

        temp.clear();
        check(temp);
        ASSERT(temp.size() == 0);
        ASSERT(! temp.contains(2));
    }
}

static void black_multiple() {
    int i;
    ISet multi3, multi4, multi5;

    check(multi3);
    multi3.insert(1);
    check(multi3);
    multi3.insert(2);
    check(multi3);
    multi3.insert(3);
    check(multi3);

    check(multi4);
    multi4.insert(1);
    check(multi4);
    multi4.insert(2);
    check(multi4);
    multi4.insert(3);
    check(multi4);
    multi4.insert(4);
    check(multi4);

    check(multi5);
    multi5.insert(1);
    check(multi5);
    multi5.insert(2);
    check(multi5);
    multi5.insert(3);
    check(multi5);
    multi5.insert(4);
    check(multi5);
    multi5.insert(5);
    check(multi5);

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

    /* Check iterator */
    {
        ASSERT(num_iterations(multi3) == 3);
        ASSERT(iteration_contains(multi3, 1));
        ASSERT(iteration_contains(multi3, 2));
        ASSERT(iteration_contains(multi3, 3));

        ASSERT(num_iterations(multi4) == 4);
        ASSERT(iteration_contains(multi4, 1));
        ASSERT(iteration_contains(multi4, 2));
        ASSERT(iteration_contains(multi4, 3));
        ASSERT(iteration_contains(multi4, 4));

        ASSERT(num_iterations(multi5) == 5);
        ASSERT(iteration_contains(multi5, 1));
        ASSERT(iteration_contains(multi5, 2));
        ASSERT(iteration_contains(multi5, 3));
        ASSERT(iteration_contains(multi5, 4));
        ASSERT(iteration_contains(multi5, 5));
    }

    /* Check copy */
    {
        ISet temp1 = multi5;
        check(temp1);
        ASSERT(compare(temp1, multi5));

        temp1.remove(5);
        ISet temp2 = temp1;
        ASSERT(compare(temp2, multi4));
        ASSERT(num_iterations(temp2) == 4);

        temp2.remove(4);
        ISet temp3 = temp2;
        ASSERT(compare(temp3, multi3));
        ASSERT(num_iterations(temp3) == 3);
    }

    /* Check insert */
    {
        ISet temp;
        check(temp);
        copy(multi3, temp);
        check(temp);

        ASSERT(compare(multi3, temp));

        /* Insert existing element */
        temp.insert(2);
        check(temp);
        ASSERT(temp.size() == multi3.size());
        ASSERT(temp.contains(2));
        temp.remove(2);
        check(temp);
        temp.insert(2);
        check(temp);
        ASSERT(compare(multi3, temp));

        /* Insert non-existent element */
        copy(multi4, temp);
        check(temp);
        ASSERT(compare(multi4, temp));
        temp.insert(5);
        check(temp);
        ASSERT(compare(multi5, temp));
        temp.remove(5);
        check(temp);
        ASSERT(compare(multi4, temp));
    }

    /* Check insert */
    {
        ISet temp;
        check(temp);
        copy(multi4, temp);
        check(temp);

        ASSERT(compare(multi4, temp));
        ASSERT(temp.size() == 4);
        temp.insert(5);
        check(temp);
        ASSERT(compare(multi5, temp));

        copy(multi3, temp);
        temp.insert(4);
        check(temp);
        temp.insert(5);
        check(temp);
        ASSERT(compare(multi5, temp));
    }

    /* Check remove */
    {
        ISet temp, empty;

        /* Check removal of existing elements */
        check(temp);
        copy(multi3, temp);
        check(temp);
        ASSERT(compare(multi3, temp));
        temp.remove(1);
        check(temp);
        temp.remove(2);
        check(temp);
        temp.remove(3);
        check(temp);
        ASSERT(compare(empty, temp));
        ASSERT(num_iterations(temp) == 0);

        copy(multi3, temp);
        check(temp);
        ASSERT(compare(multi3, temp));
        temp.remove(3);
        check(temp);
        temp.remove(2);
        check(temp);
        temp.remove(1);
        check(temp);
        ASSERT(compare(empty, temp));
        ASSERT(num_iterations(temp) == 0);

        copy(multi5, temp);
        check(temp);
        ASSERT(compare(multi5, temp));
        temp.remove(5);
        check(temp);
        ASSERT(compare(multi4, temp));
        temp.remove(4);
        check(temp);
        ASSERT(compare(multi3, temp));
        temp.remove(1);
        check(temp);
        temp.remove(2);
        check(temp);
        temp.remove(3);
        check(temp);
        ASSERT(compare(empty, temp));
        ASSERT(num_iterations(temp) == 0);

        /* Check removal of non-existent elements */
        copy(multi4, temp);
        check(temp);
        for (i = -5; i <= 0; i++) {
            temp.remove(i);
            check(temp);
            ASSERT(compare(multi4, temp));
        }
        for (i = 5; i <= 10; i++) {
            temp.remove(i);
            check(temp);
            ASSERT(compare(multi4, temp));
        }
    }

    /* Check large number of entries */
    {
        ISet set;

        check(set);
        for (i = 0; i < 1000; i++) {
            set.insert(i);
            ASSERT(num_iterations(set) == i+1);
        }
        check(set);

        for (i = 0; i < 1000; i++) {
            ASSERT(set.contains(i));
        }

        for (i = 0; i < 1000; i++) {
            set.remove(i);
            ASSERT(num_iterations(set) == (999-i));
        }
        check(set);
    }

    /* Check prediction */
    {
        //ISet set(1000);
        ISet set;

        check(set);
        for (i = 0; i < 1000; i++) {
            set.insert(i);
            ASSERT(num_iterations(set) == i+1);
        }
        check(set);

        for (i = 0; i < 1000; i++) {
            ASSERT(set.contains(i));
        }

        for (i = 0; i < 1000; i++) {
            set.remove(i);
            ASSERT(num_iterations(set) == (999-i));
        }
        check(set);
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
