/* Copyright (c) 1995 by Sanjay Ghemawat */
#include <assert.h>
#include <stdio.h>
#include "bitvec.h"

extern "C" int random();

//#define LOG_ASSERTS

#ifdef LOG_ASSERTS
#define ASSERT(x) do {fprintf(stderr, "ASSERT: %s\n", #x);assert(x);} while (0)
#else
#define ASSERT(x) assert(x)
#endif

static void check_ops(int size) {
    fprintf(stderr, "CHECK: %d\n", size);

    // Empty vector
    {
        BitVec v(size);

        for (int i = 0; i < size; i++)
            ASSERT(!v.get(i));

        ASSERT(v.first_set() == size);
        ASSERT(v.first_clear() == 0);
        ASSERT(v.length() == size);
        ASSERT(v.empty());

        v.invert();
        ASSERT(v.first_set() == 0);
        ASSERT(v.first_clear() == size);
    }

    if (size == 0) return;

    // Singleton vector
    {
        BitVec v(size);

        for (int i = 0; i < size; i++) {
            v.clear();
            ASSERT(!v.get(i));
            v.set(i);
            ASSERT(v.get(i));

            ASSERT(v.first_set() == i);
            ASSERT(v.first_clear() == ((i == 0) ? 1 : 0));
            ASSERT(!v.empty());

            v.invert();
            ASSERT(v.first_clear() == i);
            ASSERT(v.first_set() == ((i == 0) ? 1 : 0));
            v.invert();

            v.clear(i);
            ASSERT(v.first_set() == size);
            ASSERT(v.empty());
        }
    }

    // Double vector
    {
        BitVec v(size);

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (i >= j) continue;

                v.clear();
                ASSERT(!v.get(i));
                ASSERT(!v.get(j));
                v.set(i);
                ASSERT(v.get(i));
                ASSERT(!v.get(j));
                v.set(j);
                ASSERT(v.get(i));
                ASSERT(v.get(j));
                ASSERT(!v.empty());

                ASSERT(v.first_set() == i);
                if (i == 0) {
                    if (j == i+1)
                        ASSERT(v.first_clear() == j+1);
                    else
                        ASSERT(v.first_clear() == i+1);
                }
                else
                    ASSERT(v.first_clear() == 0);

                v.clear(i);
                ASSERT(v.first_set() == j);
                ASSERT(!v.empty());

                v.clear(j);
                ASSERT(v.first_set() == size);
                ASSERT(v.empty());
            }
        }
    }

    // Full vector
    {
        BitVec v(size);

        for (int i = 0; i < size; i++)
            v.set(i);

        ASSERT(v.first_set() == 0);
        ASSERT(v.first_clear() == size);
        ASSERT(!v.empty());

        for (i = 0; i < size; i++)
            ASSERT(v.get(i));

        v.invert();
        ASSERT(v.empty());
        ASSERT(v.first_set() == size);
        ASSERT(v.first_clear() == 0);
    }
}

main() {
    int i;

    for (i = 0; i < 100; i++)
        check_ops(i);

    for (i = 0; i < 10; i++)
        check_ops((random() % 200) + 100);

    return 0;
}
