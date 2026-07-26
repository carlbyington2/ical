/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _ICAL_H
#define _ICAL_H

#include <tcl.h>

// Proc for triggering tcl code based on changes
// to calendars/items.

// effects      Trigger tcl code that is waiting for triggers of type
//              "ttype".  If "id" is non-null, append it to the
//              appropriate trigger commands before executing them.
//              See "trigger.tcl".
//
//      Useful trigger types
//              add             - item was added
//              delete          - item was deleted
//              change          - item was modified
//              text            - just the text field of an item was changed
//              flush           - lots of items were added/modified/deleted...

extern void trigger(Tcl_Interp*, char const* ttype, char const* id = 0);

// Miscellaneous TCL support.

#define TCL_Return(tcl,str)                     \
    do {                                        \
        Tcl_SetResult(tcl, (char *)(str), TCL_STATIC);  \
        return TCL_OK;                          \
    } while (0)

#define TCL_Error(tcl,str)                      \
    do {                                        \
        Tcl_SetResult(tcl, (char *)(str), TCL_STATIC);  \
        return TCL_ERROR;                       \
    } while (0)


// Prototype for Tcl commands.

extern int Cmd_CreateCalendar   (ClientData, Tcl_Interp*, int, const char*[]);
extern int Cmd_CreateNotice     (ClientData, Tcl_Interp*, int, const char*[]);
extern int Cmd_CreateAppt       (ClientData, Tcl_Interp*, int, const char*[]);

extern int Cmd_Date             (ClientData, Tcl_Interp*, int, const char*[]);
extern int Cmd_Time             (ClientData, Tcl_Interp*, int, const char*[]);

/* Speedup routines. */
extern int Cmd_MonthDays        (ClientData, Tcl_Interp*, int, const char*[]);
extern int Cmd_HiliteLoop       (ClientData, Tcl_Interp*, int, const char*[]);

/* Convenience routines */
extern int Cmd_ExpandFileName   (ClientData, Tcl_Interp*, int, const char*[]);

#endif /* _ICAL_H */
