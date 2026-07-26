/* Copyright (c) 1995 by Sanjay Ghemawat */
/*
 * Test open hash set.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ohashset.h"
#include "hashfuncs.h"

declareOpenHashSet(ISet,int,hash_int,cmp_int)
implementOpenHashSet(ISet,int,hash_int,cmp_int)

//#define LOG_ASSERTS

#ifdef LOG_ASSERTS
#define ASSERT(x) do {fprintf(stderr, "ASSERT: %s\n", #x);assert(x);} while (0)
#else
#define ASSERT(x) assert(x)
#endif

/*
 * Check if M2 is a subset of M1.
 */
static int subset(ISet& m1, ISet& m2) {
    for (ISet_Elements i = &m1; i.ok(); i.next()) {
        if (! m2.contains(i.get())) {
            return 0;
        }
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
    for (ISet_Elements i = &m; i.ok(); i.next()) {
        count++;
    }
    return count;
}

/*
 * Check that iteration over set yields specified key,value pair
 */
static int iteration_contains(const ISet& m, int key) {
    for (ISet_Elements i = &m; i.ok(); i.next()) {
        if (i.get() == key) {
            return 1;
        }
    }
    return 0;
}

/* Check deletion from iterator */
static void check_del_iter(const ISet& m) {
    for (int n = 1; n <= m.size(); n++) {
        // Delete the first "n" elements of set via iterator
        ISet copy = m;
        int c = 0;
        for (ISet_Elements i = &copy; i.ok(); i.next()) {
            c++;
            if (c <= n) i.del();
        }
        ASSERT(copy.size() == (m.size() - n));

        // Delete the last "n" elements of set via iterator
        copy = m;
        c = m.size();
        for (ISet_Elements j = &copy; j.ok(); j.next()) {
            if (c <= n) j.del();
            c--;
        }
        ASSERT(copy.size() == (m.size() - n));
    }
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
    empty.check();

    /* Check size */
    ASSERT(empty.size() == 0);

    /* Check contains */
    for (i = -3; i <= 3; i++) {
        ASSERT(! empty.contains(i));
    }

    /* Check iterator */
    {
        check_del_iter(empty);
        ASSERT(num_iterations(empty) == 0);

        ISet_Elements iter = &empty;
        ASSERT(! iter.ok());
    }

    /* Check copy */
    {
        ISet temp = empty;
        temp.check();
        ASSERT(compare(temp, empty));
    }

    /* Check insert */
    {
        ISet single;
        single.check();
        single.insert(1);
        single.check();

        ASSERT(single.size() == 1);
        ASSERT(single.contains(1));
    }

    /* Check insert */
    {
        ISet single;
        single.check();
        single.insert(1);
        single.check();
        
        ASSERT(single.size() == 1);
        ASSERT(single.contains(1));
    }

    /* Check remove */
    {
        ISet empty2;
        empty2.check();

        ASSERT(empty2.size() == 0);
        empty2.remove(1);
        empty2.check();
        ASSERT(empty2.size() == 0);
    }
}

static void black_single() {
    /* Single element set tests */

    int i;

    ISet single;
    single.check();
    single.insert(2);
    single.check();

    ASSERT(single.size() == 1);

    /* Check contains */
    for (i = -3; i <= 3; i++) {
        ASSERT(single.contains(i) == (i == 2));
    }

    /* Check iterator */
    {
        check_del_iter(single);
        ASSERT(num_iterations(single) == 1);
        ASSERT(iteration_contains(single, 2));
    }

    /* Check copy */
    {
        ISet temp = single;
        temp.check();
        ASSERT(compare(temp, single));

        temp.remove(2);
        ASSERT(temp.size() == 0);

        ISet temp2 = temp;
        ASSERT(temp2.size() == 0);
    }

    /* Check insert */
    {
        ISet temp;
        temp.check();
        copy(single, temp);
        temp.check();

        ASSERT(temp.size() == 1);
        temp.insert(2);
        temp.check();
        ASSERT(temp.size() == 1);
        ASSERT(temp.contains(2));

        copy(single, temp);
        ASSERT(temp.size() == 1);
        temp.insert(3);
        temp.check();
        ASSERT(temp.size() == 2);
        ASSERT(temp.contains(2));
        ASSERT(temp.contains(3));

        ASSERT(num_iterations(temp) == 2);
        ASSERT(iteration_contains(temp, 2));
        ASSERT(iteration_contains(temp, 3));
    }

    /* Check insert */
    {
        ISet temp;
        temp.check();
        copy(single, temp);
        temp.check();

        ASSERT(temp.size() == 1);
        temp.insert(3);
        temp.check();
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
    ISet multi3, multi4, multi5;

    multi3.check();
    multi3.insert(1);
    multi3.check();
    multi3.insert(2);
    multi3.check();
    multi3.insert(3);
    multi3.check();

    multi4.check();
    multi4.insert(1);
    multi4.check();
    multi4.insert(2);
    multi4.check();
    multi4.insert(3);
    multi4.check();
    multi4.insert(4);
    multi4.check();

    multi5.check();
    multi5.insert(1);
    multi5.check();
    multi5.insert(2);
    multi5.check();
    multi5.insert(3);
    multi5.check();
    multi5.insert(4);
    multi5.check();
    multi5.insert(5);
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

    /* Check iterator */
    {
        check_del_iter(multi3);
        check_del_iter(multi4);
        check_del_iter(multi5);

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
        temp1.check();
        ASSERT(compare(temp1, multi5));

        temp1.remove(5);
        ISet temp2 = temp1;
        ASSERT(compare(temp2, multi4));

        temp2.remove(4);
        ISet temp3 = temp2;
        ASSERT(compare(temp3, multi3));
    }

    /* Check insert */
    {
        ISet temp;
        temp.check();
        copy(multi3, temp);
        temp.check();

        ASSERT(compare(multi3, temp));

        /* Insert existing element */
        temp.insert(2);
        temp.check();
        ASSERT(temp.size() == multi3.size());
        ASSERT(temp.contains(2));
        temp.remove(2);
        temp.check();
        temp.insert(2);
        temp.check();
        ASSERT(compare(multi3, temp));

        /* Insert non-existent element */
        copy(multi4, temp);
        temp.check();
        ASSERT(compare(multi4, temp));
        temp.insert(5);
        temp.check();
        ASSERT(compare(multi5, temp));
        temp.remove(5);
        temp.check();
        ASSERT(compare(multi4, temp));
    }

    /* Check insert */
    {
        ISet temp;
        temp.check();
        copy(multi4, temp);
        temp.check();

        ASSERT(compare(multi4, temp));
        ASSERT(temp.size() == 4);
        temp.insert(5);
        temp.check();
        ASSERT(compare(multi5, temp));

        copy(multi3, temp);
        temp.insert(4);
        temp.check();
        temp.insert(5);
        temp.check();
        ASSERT(compare(multi5, temp));
    }

    /* Check remove */
    {
        ISet temp, empty;

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
        ISet set;

        set.check();
        for (i = 0; i < 1000; i++) {
            set.insert(i);
            ASSERT(num_iterations(set) == i+1);
        }
        set.check();

        for (i = 0; i < 1000; i++) {
            ASSERT(set.contains(i));
        }

        for (i = 0; i < 1000; i++) {
            set.remove(i);
            ASSERT(num_iterations(set) == (999-i));
        }
        set.check();
    }

    /* Check prediction */
    {
        ISet set(1000);

        set.check();
        for (i = 0; i < 1000; i++) {
            set.insert(i);
            ASSERT(num_iterations(set) == i+1);
        }
        set.check();

        for (i = 0; i < 1000; i++) {
            ASSERT(set.contains(i));
        }

        for (i = 0; i < 1000; i++) {
            set.remove(i);
            ASSERT(num_iterations(set) == (999-i));
        }
        set.check();
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
