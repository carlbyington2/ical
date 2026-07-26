/* Copyright (c) 1993 by Sanjay Ghemawat */
/*
 * Routines to speed up dateeditor.
 */

/* Some versions of tcl.h do not seem to handle varargs correctly */
#if !defined(__STDC__) && !defined(HAS_STDARG)
#define HAS_STDARG
#endif

#include <stdio.h>
#include <string.h>
#include <tcl.h>

#include "Array.h"
#include "Date.h"
#include "Month.h"
#include "WeekDay.h"

#include "cal_tcl.h"
#include "calendar.h"
#include "calfile.h"
#include "collect.h"
#include "ical.h"
#include "item_tcl.h"

declareArray(HiliteList,char const*)
implementArray(HiliteList,char const*)

static int monday_first(Calendar_Tcl*);
static int contains(HiliteList const&, char const*);

static Calendar_Tcl* find_cal(Tcl_Interp*, char const* name);

/*
 * requires     argc/argv === <cmd> <cal> <canvas> <date>
 *              Canvas has 42 text items with tags ranging from
 *              =1 to =42.  The text items are arranged in a grid
 *              7 wide by 6 high.  The numbers increase from left to
 *              right and then top to bottom.  Therefore, the top-left
 *              item has tag =1, rop-right item has tag =7, and the
 *              bottom-right item tag =42.  Each one of these 42 items
 *              also has the tag "Day".
 *
 * effects      Configures items so that their text contents correspond
 *              to the day of the month that should be displayed at that
 *              item (under the assumption that the month  named by <date>
 *              is being displayed).
 */
int Cmd_MonthDays(ClientData, Tcl_Interp* tcl, int argc, const char* argv[]) {
    if (argc != 4) {
        TCL_Error(tcl, "illegal number of arguments");
    }

    Calendar_Tcl* cal = find_cal(tcl, argv[1]);
    if (cal == 0) {TCL_Error(tcl, "illegal calendar");}

    int dateDays;
    if (Tcl_GetInt(tcl, argv[3], &dateDays) != TCL_OK) {
        return TCL_ERROR;
    }
    Date date(dateDays);
    Date first(1, date.GetMonth(), date.GetYear());

    int start = first.GetWDay().Index();
    if (monday_first(cal)) {
        start = ((start + 5)%7) + 1;
    }
    int finish = start + first.GetMonth().Size(first.GetYear()) - 1;

    const char* canvas = argv[2];
    if (Tcl_VarEval(tcl, canvas, " itemconfig Day -text {}", NULL) != TCL_OK) {
        return TCL_ERROR;
    }

    for (int i = start; i <= finish; i++) {
        char str[100];
        sprintf(str, "=%d -text {%4d}", i, i - start + 1);
        if (Tcl_VarEval(tcl, canvas, " itemconfig ", str, NULL) != TCL_OK) {
            return TCL_ERROR;
        }
    }

    TCL_Return(tcl, "");
}

/*
 * requires     "argv == <cmd> <cal> <hlist> <start> <finish>
 *                       <dvar> <hvar> <body>"
 * effects      For each item occurrence in "<start>..<finish>",
 *              set "<dvar>" to item date, "<hvar>" to hilite string for
 *              item, and execute "<body>".  If the item hilite is not in
 *              "<hlist>", the string "always" is used instead of actual
 *              hilite style.
 */
int Cmd_HiliteLoop(ClientData, Tcl_Interp* tcl, int argc, const char* argv[]) {
    int i;

    if (argc != 8) {
        TCL_Error(tcl, "illegal number of arguments");
    }

    Calendar_Tcl* cal = find_cal(tcl, argv[1]);
    if (cal == 0) {TCL_Error(tcl, "illegal calendar");}

    int startDays;
    if (Tcl_GetInt(tcl, argv[3], &startDays) != TCL_OK) {
        return TCL_ERROR;
    }
    int finishDays;
    if (Tcl_GetInt(tcl, argv[4], &finishDays) != TCL_OK) {
        return TCL_ERROR;
    }

    int count;
    const char** strlist;
    if (Tcl_SplitList(tcl, argv[2], &count, &strlist) != TCL_OK) {
        return TCL_ERROR;
    }
    HiliteList hlist;
    for (i = 0; i < count; i++)
        hlist.append(strlist[i]);

    Date today = Date::Today();
    Date start(startDays);
    Date finish(finishDays);
    const char* dvar = argv[5];
    const char* hvar = argv[6];
    const char* body = argv[7];

    // Get all items
    ItemList items;
    collect_all(cal, items, true);

    // Get all occurrences in desired range
    Occurrences list;
    collect_occurrences(cal, items, list, start, finish, 0);

    for (i = 0; i < list.size(); i++) {
        char const* hilite = list[i].item->value()->Hilite();
        if (strcmp(hilite, "never") == 0) continue;
        if (strcmp(hilite, "expire") == 0) {
            if (list[i].date < today) continue;
            hilite = "always";
        }
        if (!contains(hlist, hilite)) hilite = "always";

        char buffer[20];
        sprintf(buffer, "%d", list[i].date.EpochDays());
        if (Tcl_SetVar(tcl, dvar, buffer, 0) == NULL) {
            Tcl_Free((char*) strlist);
            TCL_Error(tcl, "could not set loop variable");
        }

        if (Tcl_SetVar(tcl, hvar, (char*)hilite, 0) == NULL) {
            Tcl_Free((char*) strlist);
            TCL_Error(tcl, "could not set loop variable");
        }


        int result = Tcl_Eval(tcl, body);

        if (result == TCL_OK) continue;
        if (result == TCL_CONTINUE) continue;
        if (result == TCL_BREAK) break;

        // Error of some sort
        Tcl_Free((char*) strlist);
        return result;
    }

    Tcl_Free((char*) strlist);
    TCL_Return(tcl, "");
}

static int monday_first(Calendar_Tcl* cal) {
    Calendar* c = cal->main->GetCalendar();
    return ((strcmp(c->GetOption("MondayFirst"), "1") == 0) ? 1 : 0);
}

static int contains(HiliteList const& list, char const* hilite) {
    for (int i = 0; i < list.size(); i++) {
        if (strcmp(list[i], hilite) == 0) return 1;
    }
    return 0;
}

static Calendar_Tcl* find_cal(Tcl_Interp* tcl, char const* name) {
    Tcl_CmdInfo info;

    if (! Tcl_GetCommandInfo(tcl, (char*)name, &info)) return 0;
    return ((Calendar_Tcl*) info.clientData);
}
