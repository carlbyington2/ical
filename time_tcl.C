/* Copyright (c) 1993 by Sanjay Ghemawat */
/*
 * Time/Date interface for Tcl.
 */

#include <stdio.h>
#include <string.h>

extern "C" {
#include <tcl.h>
}

#include "Date.h"
#include "Time.h"

#include "Month.h"
#include "WeekDay.h"
#include "Year.h"
#include "parse.h"

#include "ical.h"

/*
 * Tcl representations for time/date entities
 *
 *	Month    is an integer in range 1..12
 *	Monthday is an integer in range 1..31
 *	Weekday  is an integer in range 1..7 (1 == Sunday)
 *	Year     is an integer in some range (possibly 1900..2400)
 *	Date     is an integer since some unspecified date)
 *	Time     is a double in seconds since some unspecified date
 */

/* Date command */
int Cmd_Date(ClientData, Tcl_Interp* tcl, int argc, char* argv[]) {
    char* cmd = argv[1];
    char buffer[100];
    int days;
    Date d;

    switch (argc) {
      case 2:
	if ((cmd[0] == 't') && (strcmp(cmd, "today") == 0)) {
	    sprintf(buffer, "%ld", Date::Today().EpochDays());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'f') && (strcmp(cmd, "first") == 0)) {
	    sprintf(buffer, "%ld", Date::First().EpochDays());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'l') && (strcmp(cmd, "last") == 0)) {
	    sprintf(buffer, "%ld", Date::Last().EpochDays());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	break;
      case 3:
	/* Argv[2] is date. */
	if (Tcl_GetInt(tcl, argv[2], &days) != TCL_OK) {
	    return TCL_ERROR;
	}
	d = Date(days);

	if ((cmd[0] == 'm') && (strcmp(cmd, "monthsize") == 0)) {
	    sprintf(buffer, "%d", d.GetMonth().Size(d.GetYear()));
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'm') && (strcmp(cmd, "monthday") == 0)) {
	    sprintf(buffer, "%d", d.GetMDay());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'w') && (strcmp(cmd, "weekday") == 0)) {
	    sprintf(buffer, "%d", d.GetWDay().Index());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'm') && (strcmp(cmd, "month") == 0)) {
	    sprintf(buffer, "%d", d.GetMonth().Index());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'y') && (strcmp(cmd, "year") == 0)) {
	    sprintf(buffer, "%d", d.GetYear());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 's') && (strcmp(cmd, "split") == 0)) {
	    int mday, year;
	    Month month;
	    WeekDay wday;

	    d.BreakDown(mday, wday, month, year);
	    sprintf(buffer, "%d %d %d %d",
		    mday,
		    wday.Index(),
		    month.Index(),
		    year);
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	break;
      case 5:
	if ((cmd[0] == 'm') && (strcmp(cmd, "make") == 0)) {
	    int month, year;

	    if ((Tcl_GetInt(tcl, argv[2], &days) != TCL_OK) ||
		(days < 1) ||
		(days > 31) ||
		(Tcl_GetInt(tcl, argv[3], &month) != TCL_OK) ||
		(month < 1) ||
		(month > 12) ||
		(Tcl_GetInt(tcl, argv[4], &year) != TCL_OK) ||
		(year < Year::First()) ||
		(year > Year::Last())) {
		TCL_Error(tcl, "illegal date specification");
	    }

	    d = Date(days, Month::First()+(month-1), year);
	    sprintf(buffer, "%ld", d.EpochDays());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	break;
      case 6:
	if ((cmd[0] == 'e') && (strcmp(cmd, "extract") == 0)) {
	    // "date extract <string> <datevar> <prevar> <postvar>"
	    Date result;
	    int start, length;

	    if (!find_date(argv[2], result, start, length))
		TCL_Return(tcl, "0");

	    // Set variables
	    sprintf(buffer, "%d", result.EpochDays());
	    if (Tcl_SetVar(tcl, argv[3], buffer, 0) == NULL)
		TCL_Error(tcl, "could not set <date> result");

	    char* pre = new char[start+1];
	    strncpy(pre, argv[2], start);
	    pre[start] = '\0';
	    if (Tcl_SetVar(tcl, argv[4], pre, 0) == NULL) {
		delete [] pre;
		TCL_Error(tcl, "could not set <pre> result");
	    }
	    delete [] pre;

	    if (Tcl_SetVar(tcl, argv[5], argv[2]+start+length, 0) == NULL)
		TCL_Error(tcl, "could not set <post> result");

	    TCL_Return(tcl, "1");
	}
	break;
      default:
	break;
    }

    TCL_Error(tcl, "invalid arguments to date");
}

/* Time command */
int Cmd_Time(ClientData, Tcl_Interp* tcl, int argc, char* argv[]) {
    char* cmd = argv[1];
    char buffer[100];
    double secs;
    Time t;
    int hour, minute, second, millisecond;

    switch (argc) {
      case 2:
	if ((cmd[0] == 'n') && (strcmp(cmd, "now") == 0)) {
	    sprintf(buffer, "%lf", Time::Now().EpochSeconds());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	break;
      case 3:
	/* Argv[2] is time */
	if (Tcl_GetDouble(tcl, argv[2], &secs) != TCL_OK) {
	    return TCL_ERROR;
	}
	t = Time(secs);

	if ((cmd[0] == 'd') && (strcmp(cmd, "date") == 0)) {
	    int day, year;
	    Month month;
	    WeekDay wday;
	    t.BreakDownDate(day, wday, month, year);
	    Date date(day, month, year);

	    sprintf(buffer, "%ld", date.EpochDays());
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}

	t.BreakDownClock(hour, minute, second, millisecond);
	if ((cmd[0] == 'h') && (strcmp(cmd, "hour") == 0)) {
	    sprintf(buffer, "%d", hour);
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'm') && (strcmp(cmd, "minute") == 0)) {
	    sprintf(buffer, "%d", minute);
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 's') && (strcmp(cmd, "second") == 0)) {
	    sprintf(buffer, "%d", second);
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 'm') && (strcmp(cmd, "millisecond") == 0)) {
	    sprintf(buffer, "%d", millisecond);
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	if ((cmd[0] == 's') && (strcmp(cmd, "split") == 0)) {
	    sprintf(buffer, "%d %d %d %d",
		    hour,
		    minute,
		    second,
		    millisecond);
	    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
	    return TCL_OK;
	}
	break;
      case 6:
	if ((cmd[0] == 'e') && (strcmp(cmd, "extract_time") == 0)) {
	    // "date extract_time <string> <todvar> <prevar> <postvar>"
	    int result;
	    int start, length;

	    if (!find_timeofday(argv[2], result, start, length))
		TCL_Return(tcl, "0");

	    // Set variables
	    sprintf(buffer, "%d", result);
	    if (Tcl_SetVar(tcl, argv[3], buffer, 0) == NULL)
		TCL_Error(tcl, "could not set <time of day> result");

	    char* pre = new char[start+1];
	    strncpy(pre, argv[2], start);
	    pre[start] = '\0';
	    if (Tcl_SetVar(tcl, argv[4], pre, 0) == NULL) {
		delete [] pre;
		TCL_Error(tcl, "could not set <pre> result");
	    }
	    delete [] pre;

	    if (Tcl_SetVar(tcl, argv[5], argv[2]+start+length, 0) == NULL)
		TCL_Error(tcl, "could not set <post> result");

	    TCL_Return(tcl, "1");
	}
	break;
      case 7:
	if ((cmd[0] == 'e') && (strcmp(cmd, "extract_range") == 0)) {
	    // "date extract_range <string> <startvar> <finishvar>\
	    //			   <prevar> <postvar>"
	    int r1, r2;
	    int start, length;

	    if (!find_timerange(argv[2], r1, r2, start, length))
		TCL_Return(tcl, "0");

	    // Set variables
	    sprintf(buffer, "%d", r1);
	    if (Tcl_SetVar(tcl, argv[3], buffer, 0) == NULL)
		TCL_Error(tcl, "could not set <start time> result");

	    // Set variables
	    sprintf(buffer, "%d", r2);
	    if (Tcl_SetVar(tcl, argv[4], buffer, 0) == NULL)
		TCL_Error(tcl, "could not set <finish time> result");

	    char* pre = new char[start+1];
	    strncpy(pre, argv[2], start);
	    pre[start] = '\0';
	    if (Tcl_SetVar(tcl, argv[5], pre, 0) == NULL) {
		delete [] pre;
		TCL_Error(tcl, "could not set <pre> result");
	    }
	    delete [] pre;

	    if (Tcl_SetVar(tcl, argv[6], argv[2]+start+length, 0) == NULL)
		TCL_Error(tcl, "could not set <post> result");

	    TCL_Return(tcl, "1");
	}
	break;
      default:
	break;
    }

    TCL_Error(tcl, "invalid arguments to time");
}
