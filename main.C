/* Copyright (c) 1994 by Sanjay Ghemawat */
/*
 * Behaves like Tk main routine if display can be opened, otherwise
 * like the Tcl main routine.  The following control whether or not
 * Tk is used --
 *
 *      $DISPLAY in environment                 Use Tk
 *      -display                                Use Tk
 *      -list                                   Do not use Tk
 *      -show                                   Do not use Tk
 *      -print                                  Do not use Tk
 *      -nodisplay                              Do not use Tk
 *
 * The "-f" flag can be used to pass in an initialization script regardless
 * of whether or not Tk is used.
 *
 * All .tcl files from Tcl/Tk libraries are linked into the executable
 * as well to avoid depending on external files being installed correctly.
 */

#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#include "ical.h"

#include "bitmaps/left.xbm"
#include "bitmaps/right.xbm"
#include "bitmaps/todo.xbm"
#include "bitmaps/done.xbm"
#include "bitmaps/sleft.xbm"
#include "bitmaps/dleft.xbm"
#include "bitmaps/sright.xbm"
#include "bitmaps/dright.xbm"
#include "bitmaps/ical.xbm"

// Is Tk available?
static int have_tk;

// Was a script specified on the command line?
static int have_script;

static int eval_list(Tcl_Interp*, const char** list);
static int app_init(Tcl_Interp*);
extern int Ical_Init(Tcl_Interp*);

int
main(int argc, char* argv[]) {
    // XXX Hacky scanning of argument list to figure out whether
    // or not Tk is needed, and also if a script is specified on the
    // command line.

    have_script = 0;
    have_tk = (getenv("DISPLAY") != 0);

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-display") == 0) {
            have_tk = 1;
            continue;
        }
        if (strcmp(argv[i], "-list") == 0) {
            have_tk = 0;
            continue;
        }
        if (strcmp(argv[i], "-show") == 0) {
            have_tk = 0;
            continue;
        }
        if (strcmp(argv[i], "-print") == 0) {
            have_tk = 0;
            continue;
        }
        if (strcmp(argv[i], "-nodisplay") == 0) {
            have_tk = 0;
            continue;
        }
        if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "-file") == 0)) {
            have_script = 1;
            continue;
        }
    }

    // Strip out processed "-nodisplay" arguments
    int j = 1;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-nodisplay") == 0) continue;
        argv[j++] = argv[i];
    }
    argv[j] = 0;
    argc = j;

    if (!have_tk && have_script) {
        // If a "-f <script>" is present on the command line,
        // strip out the "-f" because tclMain does not understand it.
        for (i = 1; i < argc-1; i++) {
            if ((strcmp(argv[i],"-f") != 0) && (strcmp(argv[i],"-file") != 0))
                continue;

            /* Slide the rest of the arguments over */
            /* (including the NULL in argv[argc].   */

            for (int j = i+1; j <= argc; j++)
                argv[j-1] = argv[j];
            argc--;
            break;
        }
    }


    if (have_tk)
        Tk_Main(argc, argv, app_init);
    else
        Tcl_Main(argc, argv, app_init);

    return 0;
}

static int app_init(Tcl_Interp* tcl) {
    if (Tcl_Init(tcl) != TCL_OK) return TCL_ERROR;
    if (have_tk && (Tk_Init(tcl) != TCL_OK)) return TCL_ERROR;
    if (Ical_Init(tcl) != TCL_OK) return TCL_ERROR;

    if (!have_script) {
        // Perform default initialization
        if (have_tk) {
            if (Tcl_Eval(tcl, "ical_tk_script") == TCL_ERROR)
                return TCL_ERROR;

            // Do not bother returning to tkMain because it
            // will try to read from standard input.

            Tk_MainLoop();
            Tcl_Eval(tcl, "exit");
            exit(1);
        }

        // Default tcl code
        return Tcl_Eval(tcl, "ical_no_tk_script");
    }

    return TCL_OK;
}

// Macro to create a Tk bitmap.  Returns true iff successfull.
#define MAKE_BITMAP(tcl,id,n) \
(Tk_DefineBitmap(tcl,Tk_GetUid(id),n##_bits,n##_width,n##_height) == TCL_OK)

int Ical_Init(Tcl_Interp* tcl) {
    if (have_tk) {
        /* Load necessary Tk support code */
        Tk_Window mainWindow = Tk_MainWindow(tcl);

        if (!MAKE_BITMAP(tcl, "left_arrow",     left))    return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "right_arrow",    right))   return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "todo_box",       todo))    return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "done_box",       done))    return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "single_left",    sleft))   return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "double_left",    dleft))   return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "single_right",   sright))  return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "double_right",   dright))  return TCL_ERROR;
        if (!MAKE_BITMAP(tcl, "ical_icon",      ical))    return TCL_ERROR;
    }

    // Non-Tk ical commands
    Tcl_CreateCommand(tcl, "calendar",     Cmd_CreateCalendar,  NULL, NULL);
    Tcl_CreateCommand(tcl, "notice",       Cmd_CreateNotice,    NULL, NULL);
    Tcl_CreateCommand(tcl, "appointment",  Cmd_CreateAppt,      NULL, NULL);
    Tcl_CreateCommand(tcl, "date",         Cmd_Date,            NULL, NULL);
    Tcl_CreateCommand(tcl, "ical_time",    Cmd_Time,            NULL, NULL);
    Tcl_CreateCommand(tcl, "de_monthdays", Cmd_MonthDays,       NULL, NULL);
    Tcl_CreateCommand(tcl, "hilite_loop",  Cmd_HiliteLoop,      NULL, NULL);
    Tcl_CreateCommand(tcl, "ical_expand_file_name", Cmd_ExpandFileName, 0, 0);

    // Initialize ical stuff
    if (Tcl_EvalFile(tcl, ICALLIBDIR "/startup.tcl") != TCL_OK)
        return TCL_ERROR;

    if (Tcl_Eval(tcl, "ical_init") == TCL_ERROR)
        return TCL_ERROR;

    return TCL_OK;
}

// Concatenate list of lines into one string and "Tcl_Eval" it.
static int eval_list(Tcl_Interp* tcl, const char** list) {
    // Get buffer size
    int i;
    int count = 0;
    for (i = 0; list[i] != 0; i++) {
        count += strlen(list[i]);
        count++;                        // Space for newline
    }

    // Copy lines into buffer
    int index = 0;
    char* buf = new char[count+1];
    for (i = 0; list[i] != 0; i++) {
        strcpy(buf+index, list[i]);
        index += strlen(list[i]);
        buf[index] = '\n';
        index++;
    }
    buf[index] = '\0';

    int result = Tcl_Eval(tcl, buf);
    delete [] buf;
    return result;
}
