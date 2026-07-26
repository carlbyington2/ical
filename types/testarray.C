/* Copyright (c) 1995 by Sanjay Ghemawat */
/*
Tests for arrays.

You can supply an optional integer argument to repeat the
test a certain number of times.  This feature can be used
to measure the cost of array operations.
*/

#include <stdlib.h>
#include <stdio.h>
#include "Array.h"

declareArray(array,int)
implementArray(array,int)

static void compare(int i1, int i2) {
    if (i1 != i2) {
        fprintf(stderr, "int mismatch: %d -> %d\n", i1, i2);
        exit(1);
    }
}

/* Compare two arrays. */
static void compare(array& a1, array& a2) {
    compare(a1.size(), a2.size());
    for (int i = 0; i < a1.size(); i++) {
        compare(a1[i], a2[i]);
    }
}

static void run_test() {
    int i;

    /*
     * Some basic reference arrays.
     */
    array zero10 = array(0, 10);
    array empty1 = array();
    array empty2 = array(100);

    array increasing;
    for (i = 1; i <= 10; i++) {
        increasing.append(3 * i);
    }

    array decreasing;
    for (i = 1; i <= 10; i++) {
        decreasing.append(2 * (11 - i));
    }

    /* test reference arrays. */
    {
        compare(empty1, empty2);

        for (i = 0; i < zero10.size(); i++) {
            compare(0, zero10[i]);
        }

        compare(10, increasing.size());
        for (i = 1; i <= increasing.size(); i++) {
            compare(3 * i, increasing[i-1]);
        }

        compare(10, decreasing.size());
        for (i = 1; i <= decreasing.size(); i++) {
            compare(2 * (11 - i), decreasing[i-1]);
        }
    }

    /* test constructors */
    {
        array t1 = array(increasing);
        compare(t1, increasing);

        array t2 = array(increasing.as_pointer(), increasing.size());
        compare(t2, increasing);
    }

    /* test assignment */
    {
        array t1 = array(increasing);
        t1 = zero10;
        compare(t1,zero10);
        t1 = empty1;
        compare(t1, empty1);
        compare(t1, empty2);
    }

    /* test indexing */
    {
        array t1 = array(increasing);
        for (i = 0; i < t1.size(); i++) {
            compare(t1.slot(i), increasing[i]);
            compare(t1[i], increasing.slot(i));
        }
    }

    /* test miscellaneous */
    {
        array t1 = array(zero10);
        compare(t1.size(), 10);

        t1 = increasing;
        compare(t1[t1.size()-1], t1.high());
        compare(t1.high(), increasing.high());
    }

    /* test growth */
    {
        array t1 = array(9, 17);
        array t2;
        for (i = 0; i < t1.size(); i++) {
            t2.append(t1[i]);
        }
        compare(t1, t2);

        array t3 = array(t1);
        t1.concat(decreasing);
        t2.concat(decreasing.as_pointer(), decreasing.size());
        t3.concat(decreasing);
        compare(t1, t2);
        compare(t2, t3);
    }

    /* test remove */
    {
        array t1 = array(zero10);

        array t2;
        for (int i = 0; i < (t1.size() - 3); i++)
            t2.append(t1[i]);

        t1.remove();
        t1.remove();
        t1.remove();
        compare(t1, t2);

        array t3 = array(zero10);
        t3.remove(3);
        compare(t2, t3);
    }

    /* test clear */
    {
        array t1 = array(increasing);
        t1.concat(decreasing);
        t1.concat(zero10);

        array t2 = array(t1);

        while (t1.size() > 0)
            t1.remove();

        t2.clear();

        compare(t1, empty1);
        compare(t2, empty1);
    }

    /* test reclaim */
    {
        array t1 = array(0, 100);
        array t2 = array();
        array t3 = array(100);

        t1.remove(t1.size() - zero10.size());
        t1.reclaim();
        t2.reclaim();
        t3.reclaim();

        compare(t1, zero10);
        compare(t2, empty1);
        compare(t3, empty2);
    }
}

static void usage() {
    fprintf(stderr, "Usage: testarray [<number of repetitions>]\n");
    exit(1);
}

int
main(int argc, char* argv[]) {
    if (argc > 2) {
        usage();
    }

    int count = 1;
    if (argc == 2) {
        char* result;

        count = strtol(argv[1], &result, 10);
        if ((result == argv[1]) || (count < 0)) {
            usage();
        }
    }

    for (int i = 0; i < count; i++) {
        run_test();
    }

    return 0;
}
