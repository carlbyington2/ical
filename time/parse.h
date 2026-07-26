/* Copyright (c) 1994 by Sanjay Ghemawat */

#ifndef _PARSE_H
#define _PARSE_H

#include "Date.h"
#include "Time_.h"

// Routines to parse dates and times

extern int find_date(char const* string,Date& date,int& start,int& length);
        // modifies     "date", "start", "finish".
        // effects      Returns true iff a date is found in "string".
        //              If a date is found, it is parsed and stored into
        //              "date", and "start"/"length" are set to the
        //              range of characters occupied by the date
        //              specification.
        //
        //              I.e., "start" and "length" are modified so that
        //              the parsed date occupies
        //                      "string[start..(start+length-1)]"

extern int find_timeofday(char const* string,int& time,int& start,int& length);
        // modifies     "time", "start", "finish".
        // effects      Returns true iff a time of day is found in "string".
        //              If a tod is found, it is parsed and stored into
        //              "time", and "start"/"length" are set to the
        //              range of characters occupied by the time of day
        //              specification.
        //
        //              I.e., "start" and "length" are modified so that
        //              the parsed tod occupies
        //                      "string[start..(start+length-1)]"
        //
        //              The value stored into "time" is the number of seconds
        //              since midnight.


extern int find_timerange(char const* string,
                          int& starttime, int& finishtime,
                          int& start,int& length);
        // modifies     "starttime", "finishtime", "start", "finish".
        // effects      Returns true iff a time range is found in "string".
        //              If a range is found, it is parsed and stored into
        //              "starttime"/"finishtime", and "start"/"length"
        //              are set to the range of characters occupied by
        //              the time range specification.
        //
        //              I.e., "start" and "length" are modified so that
        //              the parsed range occupies
        //                      "string[start..(start+length-1)]"
        //
        //              The values stored into "starttime" and "finishtime"
        //              are seconds since midnight.

#endif /* _PARSE_H */
