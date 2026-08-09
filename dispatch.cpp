/* Copyright (c) 1993 by Sanjay Ghemawat */
#include <string.h>
#include "dispatch.h"
#include "ical.h"

int Dispatch(Dispatch_Entry* table, ClientData c, Tcl_Interp* tcl,
             int argc, const char* argv[]) {

    if (argc < 2) {
        TCL_Error(tcl, "invalid command");
    }

    const char* command = argv[1];
    argc -= 2;
    argv += 2;

    for (int i = 0; table[i].command != 0; i++) {
        Dispatch_Entry* entry = table+i;

        if (strcmp(command, entry->command) == 0) {
            if (argc < entry->min_arg_count) {
                TCL_Error(tcl, "not enough arguments");
            }
            if ((entry->max_arg_count >= 0) && (argc > entry->max_arg_count)) {
                TCL_Error(tcl, "too many arguments");
            }
            return entry->handler(c, tcl, argc, argv);
        }
    }

    TCL_Error(tcl, "unknown command");
}
