/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _DISPATCH_H
#define _DISPATCH_H

extern "C" {
#include <tcl.h>
}

/*
 * Dispatch Tcl command to appropriate procedure.
 *
 * Tcl invocation takes form --
 *
 *      <command> <subcommand> <args...>
 *
 * The dispatch is based on <subcommand>.
 */

struct Dispatch_Entry {
    char const* command;        /* Sub-command to dispatch on */

    /*
     * Argument count limits -- these counts apply after <command>
     * and <subcommand> are removed from argc/argv.
     */
    int  min_arg_count;         /* Minimum argument count */
    int  max_arg_count;         /* Maximum argument count. -1 means no max */

    /*
     * The handler procedure.  <command> and <subcommand> are stripped
     * away from argc/argv and then the handler is called.
     */
    Tcl_CmdProc* handler;
};

/*
 * The dispatcher --
 *
 * It gets an array of Dispatch_Entry as its first argument.
 * The array is delimited by a NULL command field in the last entry.
 */

extern int Dispatch(Dispatch_Entry*, ClientData, Tcl_Interp*, int, const char*[]);

#endif /* _DISPATCH_H */
