/* Copyright (c) 1993 by Sanjay Ghemawat */
#include "basic.h"
#include "cal_tcl.h"
#include "ical.h"
#include "item_tcl.h"

/*
 * Notice Creator
 *
 *      notice <name>
 */

int Cmd_CreateNotice(ClientData, Tcl_Interp* tcl, int argc, const char* argv[]) {
    if (argc != 1) {
        TCL_Error(tcl, "invalid arguments to notice");
    }

    Item_Tcl* item = new Item_Tcl(tcl, new Notice, 0);
    TCL_Return(tcl, (char*) item->handle());
}

/*
 * Appointment Creator
 *
 *      appointment <name>      -- Returns <name>
 */

int Cmd_CreateAppt(ClientData, Tcl_Interp* tcl, int argc, const char* argv[]) {
    if (argc != 1) {
        TCL_Error(tcl, "invalid arguments to appointment");
    }

    Item_Tcl* item = new Item_Tcl(tcl, new Appointment, 0);
    TCL_Return(tcl, (char*) item->handle());
}

/*
 * Calendar Creator
 *
 *      calendar <name> <filename>      -- Returns <name>
 */

int Cmd_CreateCalendar(ClientData, Tcl_Interp* tcl, int argc, const char* argv[]) {
    if (argc != 3) {
        TCL_Error(tcl, "invalid arguments to calendar");
    }

    Calendar_Tcl* c = new Calendar_Tcl(tcl, argv[1], argv[2]);
    if (!c->error())
        TCL_Return(tcl, "");

    Tcl_SetResult(tcl, (char*)c->error_msg(), TCL_VOLATILE);
    delete c;
    return TCL_ERROR;
}

/*
 * usage        ical_expand_file_name <filename>
 * effects      Converts <filename> into a name suitable for passing
 *              to the local operating system.  This transformation
 *              may involve tilde substitution.  If any part of the
 *              of the translation fails, this routine returns an
 *              error.  Otherwise it returns the translated file name.
 */
int Cmd_ExpandFileName(ClientData, Tcl_Interp* tcl, int argc, const char* argv[]) {
    if (argc != 2) {
        TCL_Error(tcl, "invalid arguments to expand_file_name");
    }

    Tcl_DString buf;
    char* result = Tcl_TranslateFileName(tcl, argv[1], &buf);
    if (result != NULL) {
        Tcl_SetResult(tcl, result, TCL_VOLATILE);
        Tcl_DStringFree(&buf);
        return TCL_OK;
    } else {
        return TCL_ERROR;
    }
}
