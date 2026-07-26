/* Copyright (c) 1996  by Sanjay Ghemawat */
// Speed tests for the hashed map

#include <iostream.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include "longmap.h"

main(int argc, char *argv[]) {
    LongMap m;
    m.predict(100);
    int size = 20000;
    int reps;
    int count = 0;
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <repetitions> [<size>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    srand48(time(0));
    reps = atoi(argv[1]);
    if (argc == 3) size = atoi(argv[2]);
    for (long i = 0; i < reps; i++) {
        long j1 = i%size, j2 = size - j1;
        m.insert(j1, i);
        m.remove(j2);
    }
    m.report_stats("table");
    return EXIT_SUCCESS;
}
