/* Copyright (c) 1994 by Sanjay Ghemawat */

#include <ctype.h>
#include <string.h>

#include "../types/Array.h"

#include "Date.h"
#include "Month.h"
#include "WeekDay.h"
#include "Year.h"
#include "config.h"

// Token types
enum Token_Type {
    INT_TOKEN,
    MONTH_TOKEN,
    WEEKDAY_TOKEN,

    TODAY_TOKEN,
    TOMORROW_TOKEN,
    YESTERDAY_TOKEN,

    AMPM_TOKEN,
    MIDNIGHT_TOKEN,
    NOON_TOKEN,
    ZONE_TOKEN,

    SLASH_TOKEN,
    COLON_TOKEN,

    TO_TOKEN,

    OTHER_TOKEN,
    LAST_TOKEN
}; 

// Part of string being parsed.
struct Token {
    char const* ptr;            // Pointer into string
    int         len;            // Length of this token
    Token_Type  ttype;          // The type of this token
    long        tvalue;         // Any value associated with this token
};

declareArray(Tokens,Token)
implementArray(Tokens,Token)

static void parse_tokens(char const*, Tokens&);
static void parse_token(char const*, Token&);
static int  parse_word(char const*, int, Token&);

static void parse_tokens(char const* str, Tokens& list) {
    while (str[0] != '\0') {
        Token t;
        parse_token(str, t);
        str += t.len;

        // Merge trailing whitespace etc. into token
        while (str[0] != '\0') {
            char c = str[0];
            if ((c != '.') && (c != ',') && !isspace(c))
                break;

            t.len++;
            str++;
        }

        list.append(t);
    }

    // Append guard token
    Token t;
    t.ptr = str;
    t.len = 0;
    t.ttype = LAST_TOKEN;
    list.append(t);
}

// effects      Parse token from string and stuff it in "t".
static void parse_token(char const* string, Token& t) {
    char c = string[0];

    // Parse number
    if (isdigit(c)) {
        char* endptr;
        long v = strtol(string, &endptr, 10);
        if (endptr > string) {
            t.ptr = string;
            t.len = endptr - string;
            t.ttype = INT_TOKEN;
            t.tvalue = v;
            
            return;
        }
    }

    // Special handling for "am/pm".
    if (isalpha(c)) {
        // Skip periods and alphabetic characters.
        int end = 1;
        while ((string[end] != '\0') &&
               (isalpha(string[end]) || (string[end] == '.')))
            end++;

        if (parse_word(string, end, t))
            return;
    }
    
    // Parse word
    if (isalpha(c)) {
        int end = 1;
        while ((string[end] != '\0') && isalpha(string[end]))
            end++;

        if (parse_word(string, end, t))
            return;

        // If word has exactly three uppercase letters, consider it a
        // time zone.
        if ((end == 3) &&
            isupper(string[0]) &&
            isupper(string[1]) &&
            isupper(string[2])) {
            t.ptr = string;
            t.len = end;
            t.ttype = ZONE_TOKEN;
            return;
        }
    }

    // Parse zone
    if ((c == '+') || (c == '-')) {
        Token rest;
        parse_token(string+1, rest);
        if ((rest.ttype == INT_TOKEN) && (rest.len >= 3)) {
            // Found a time zone
            t.ptr = string;
            t.len = rest.len+1;
            t.ttype = ZONE_TOKEN;
            return;
        }
    }

    // Parse range separator
    for (int seplen = 1; seplen <= 3; seplen++) {
        if (strncasecmp(string, "---", seplen) == 0) {
            t.ptr = string;
            t.len = seplen;
            t.ttype = TO_TOKEN;
            return;
        }
    }

    t.ptr = string;
    t.len = 1;

    switch (c) {
      case '/':
        t.ttype = SLASH_TOKEN;
        break;
      case ':':
        t.ttype = COLON_TOKEN;
        break;
      default:
        t.ttype = OTHER_TOKEN;
        break;
    }
}

// The lengths of the following tokens are the minimum lengths
// that are required for a match.
static Token words[] = {
    { "today",          5,      TODAY_TOKEN,    0       },
    { "tomorrow",       5,      TOMORROW_TOKEN, 0       },
    { "yesterday",      5,      YESTERDAY_TOKEN,0       },

    { "january",        3,      MONTH_TOKEN,    1       },
    { "february",       3,      MONTH_TOKEN,    2       },
    { "march",          3,      MONTH_TOKEN,    3       },
    { "april",          3,      MONTH_TOKEN,    4       },
    { "may",            3,      MONTH_TOKEN,    5       },
    { "june",           3,      MONTH_TOKEN,    6       },
    { "july",           3,      MONTH_TOKEN,    7       },
    { "august",         3,      MONTH_TOKEN,    8       },
    { "september",      3,      MONTH_TOKEN,    9       },
    { "october",        3,      MONTH_TOKEN,    10      },
    { "november",       3,      MONTH_TOKEN,    11      },
    { "december",       3,      MONTH_TOKEN,    12      },

    { "sunday",         3,      WEEKDAY_TOKEN,  1       },
    { "monday",         3,      WEEKDAY_TOKEN,  2       },
    { "tuesday",        3,      WEEKDAY_TOKEN,  3       },
    { "wednesday",      3,      WEEKDAY_TOKEN,  4       },
    { "thursday",       3,      WEEKDAY_TOKEN,  5       },
    { "friday",         3,      WEEKDAY_TOKEN,  6       },
    { "saturday",       3,      WEEKDAY_TOKEN,  7       },

    { "a.m.",           4,      AMPM_TOKEN,     0       },
    { "am.",            3,      AMPM_TOKEN,     0       },
    { "am",             2,      AMPM_TOKEN,     0       },
    { "p.m.",           4,      AMPM_TOKEN,     1       },
    { "pm.",            3,      AMPM_TOKEN,     1       },
    { "pm",             2,      AMPM_TOKEN,     1       },

    { "midnight",       8,      MIDNIGHT_TOKEN, 0       },
    { "noon",           4,      NOON_TOKEN,     0       },

    { "to",             2,      TO_TOKEN,       0       },

    { 0,                0,      OTHER_TOKEN,    0       }
};

static int build_date(int m, int d, int y, Date& result) {
    // Fix year
    if (y < 100) y += (Date::Today().GetYear() / 100) * 100;

    // Check range
    if ((y < Year::First()) || (y > Year::Last())) return 0;
    if ((m < 1) || (m > 12)) return 0;

    Month month = Month::First() + (m - 1);
    if ((d < 1) || (d > month.Size(y))) return 0;

    result = Date(d, month, y);
    return 1;
}

static int parse_word(char const* str, int len, Token& t) {
    for (int i = 0; words[i].ptr != 0; i++) {
        if (len < words[i].len) continue;
        if (len > strlen(words[i].ptr)) continue;
        if (strncasecmp(str, words[i].ptr, len) != 0) continue;

        // Found it
        t.ptr = str;
        t.len = len;
        t.ttype = words[i].ttype;

        t.tvalue = words[i].tvalue;
        return 1;
    }

    return 0;
}

// Used to ignore "am/pm" indicator from "match_timeofday"
static int junk_ampm;

// modifies     "result", "had_ampm"
// effects      If "list" starting at "index" contains a time specification,
//              parse the time into "result" and return the number of
//              tokens used in the parse.  Otherwise return zero.
//
//              "result" is set to the number of seconds after midnight.
//              "had_ampm" is set to true if an am/pm indication was
//              found, otherwise it is set to false.

static int match_timeofday(Tokens& list, int index, int& result,
                           int& had_ampm = junk_ampm) {
    if (index >= list.size()) return 0;
    Token t = list[index];

    if (t.ttype == MIDNIGHT_TOKEN) {
        result = 0;
        had_ampm = 1;
        return 1;
    }

    if (t.ttype == NOON_TOKEN) {
        result = 12 * 60 * 60;
        had_ampm = 1;
        return 1;
    }

    // "<hour>:<minute>[:<second>] [am|pm] [<time zone>]"
    if ((list[index+0].ttype == INT_TOKEN) &&
        (list[index+1].ttype == COLON_TOKEN) &&
        (list[index+2].ttype == INT_TOKEN)) {
        int h = list[index+0].tvalue;
        int m = list[index+2].tvalue;
        int s = 0;
        int c = 3;

        // See if seconds are specified
        if ((list[index+c+0].ttype == COLON_TOKEN) &&
            (list[index+c+1].ttype == INT_TOKEN)) {
            s = list[index+c+1].tvalue;
            c += 2;
        }

        // See if am/pm is specified
        int have_ampm = 0;
        int ispm = 0;
        if (list[index+c].ttype == AMPM_TOKEN) {
            have_ampm = 1;
            ispm = list[index+c].tvalue;
            c++;
        }

        // See if time zone is specified
        if (list[index+c].ttype == ZONE_TOKEN) {
            // XXX - Handle zone
            c++;
        }

        if (have_ampm) {
            // Hours > 12 not allowed with am/pm
            if (h > 12) return 0;

            if (h == 12) h = 0;
            if (ispm) h += 12;
        }

        // Range checking
        if (h >= 24) return 0;
        if ((m < 0) || (m > 60)) return 0;
        if ((s < 0) || (s > 60)) return 0;

        result = ((h * 60) + m) * 60 + s;
        had_ampm = (have_ampm || (h >= 12));
        return c;
    }

    // "<hour> [am|pm]"
    if ((list[index+0].ttype == INT_TOKEN) &&
        (list[index+1].ttype == AMPM_TOKEN)) {
        int h = list[index].tvalue;
        if (h > 12) return 0;
        if (h == 12) h = 0;
        if (list[index+1].tvalue) h += 12;

        result = (h * 60 * 60);
        had_ampm = 1;
        return 2;
    }

    return 0;
}

// effects      If "list" starting at "index" contains a range specification,
//              parse the range into and return the number of
//              tokens used in the parse.  Otherwise return zero.

static int match_timerange(Tokens& list, int index, int& start, int& finish) {
    if (index >= list.size()) return 0;
    Token t = list[index];
    int noon = 12 * 60 * 60;

    // "<timeofday> <to> <timeofday>"
    int c1, r1, c2, r2, ampm1, ampm2;
    if (((c1 = match_timeofday(list, index, r1, ampm1)) != 0) &&
        (list[index+c1].ttype == TO_TOKEN) &&
        ((c2 = match_timeofday(list, index+c1+1, r2, ampm2)) != 0)) {

        if (!ampm1 && !ampm2 && (r1 > r2) && (r1 < noon) && (r2 < noon)) {
            // r1 must be am and r2 must be pm
            r2 += noon;
        }
        if (ampm1 && !ampm2 && (r1 < noon) && (r2 < r1)) {
            // r2 must be pm
            r2 += noon;
        }
        if (!ampm1 && ampm2 && (r1 < noon) && (r2 >= noon) &&
            ((r1 + noon) <= r2)) {
            // Make r1 match r2
            r1 += noon;
        }

        if (r1 <= r2) {
            start = r1;
            finish = r2;
            return c1 + c2 + 1;
        }
    }

    // "<hour> <to> <timeofday>"
    if ((list[index+0].ttype == INT_TOKEN) &&
        (list[index+1].ttype == TO_TOKEN) &&
        (list[index+0].tvalue > 0) &&
        (list[index+0].tvalue <= 12) &&
        ((c2 = match_timeofday(list, index+2, r2, ampm2)) != 0)) {
        int h = list[index+0].tvalue;
        if (h == 12) h = 0;
        r1 = h * 60 * 60;

        if (!ampm2 && (r1 > r2) && (r2 < noon)) {
            // r2 must be after noon
            r2 += noon;
        }

        // Move "r1" into second half of day if range > 12 hrs
        if ((r2 - r1) > (12*60*60))
            r1 += 12*60*60;

        if (r1 <= r2) {
            start = r1;
            finish = r2;
            return c2 + 2;
        }
    }

    return 0;
}

// effects      If "list" starting at "index" contains a date specification,
//              parse the date into "date" and return the number of tokens used
//              in the parse.  Otherwise return zero.

static int match_date(Tokens& list, int index, Date& result) {
    if (index >= list.size()) return 0;

    Token t = list[index];

    // "<weekday> <date>"
    if (t.ttype == WEEKDAY_TOKEN) {
        // Try to find date starting at next token
        Date r;
        int count = match_date(list, index+1, r);
        if ((count > 0) && (r.GetWDay().Index() == t.tvalue)) {
            // Weekday matches the date
            result = r;
            return count + 1;
        }

        // Weekday does not match
        return 0;
    }

    // "today"
    if (t.ttype == TODAY_TOKEN) {
        result = Date::Today();
        return 1;
    }

    // "tomorrow"
    if (t.ttype == TOMORROW_TOKEN) {
        result = Date::Today() + 1;
        return 1;
    }

    // "yesterday"
    if (t.ttype == YESTERDAY_TOKEN) {
        result = Date::Today() - 1;
        return 1;
    }

    // "<int>/<int>[/<int>]"
    if ((list[index+0].ttype == INT_TOKEN) &&
        (list[index+1].ttype == SLASH_TOKEN) &&
        (list[index+2].ttype == INT_TOKEN)) {

        int m = list[index+0].tvalue;
        int d = list[index+2].tvalue;
        if (m > 12) {
            // European format
            int tmp = m;
            m = d;
            d = tmp;
        }

        int count;
        int y;
        if ((list[index+3].ttype == SLASH_TOKEN) &&
            (list[index+4].ttype == INT_TOKEN)) {
            // Found year as well
            y = list[index+4].tvalue;
            count = 5;
        }
        else {
            // Only month and year
            y = Date::Today().GetYear();
            count = 3;
        }

        if (build_date(m, d, y, result))
            return count;
    }

    // "<month> <int> [<time>] [<int>]"
    if ((list[index+0].ttype == MONTH_TOKEN) &&
        (list[index+1].ttype == INT_TOKEN)) {

        int m = list[index+0].tvalue;
        int d = list[index+1].tvalue;
        int y = Date::Today().GetYear();
        int c = 2;

        // Skip time specification (if any)
        int seconds;
        c += match_timeofday(list, index+c, seconds);

        // Handle year specification (if any)
        if (list[index+c].ttype == INT_TOKEN) {
            y = list[index+c].tvalue;
            c++;
        }

        if (build_date(m, d, y, result))
            return c;
    }

    // "<int> <month> [<time>] [<int>]"
    if ((list[index+0].ttype == INT_TOKEN) &&
        (list[index+1].ttype == MONTH_TOKEN)) {

        int m = list[index+1].tvalue;
        int d = list[index+0].tvalue;
        int y = Date::Today().GetYear();
        int c = 2;

        // Skip time specification (if any)
        int seconds;
        c += match_timeofday(list, index+c, seconds);

        // Handle year specification (if any)
        if (list[index+c].ttype == INT_TOKEN) {
            y = list[index+c].tvalue;
            c++;
        }

        if (build_date(m, d, y, result))
            return c;
    }

    return 0;
}

// Exported routines

int find_date(char const* string, Date& result, int& start, int& length) {
    Tokens list;
    parse_tokens(string, list);

    int index = 0;              // Index into "list"
    int pos = 0;                // Number of chars in "list[0..index-1]".
    while (index < list.size()) {
        Date d;
        int count = match_date(list, index, d);
        if (count == 0) {
            // No date here, keep searching
            pos += list[index].len;
            index++;
            continue;
        }

        // Found date at "index"
        result = d;
        start = pos;
        length = 0;
        for (int i = index; i < index + count; i++)
            length += list[i].len;

        return 1;
    }

    return 0;
}

int find_timeofday(char const* string, int& result, int& start, int& length) {
    Tokens list;
    parse_tokens(string, list);

    int index = 0;              // Index into "list"
    int pos = 0;                // Number of chars in "list[0..index-1]".
    while (index < list.size()) {
        int seconds;
        int count = match_timeofday(list, index, seconds);
        if (count == 0) {
            // No time here, keep searching
            pos += list[index].len;
            index++;
            continue;
        }

        // Found time at "index"
        result = seconds;
        start = pos;
        length = 0;
        for (int i = index; i < index + count; i++)
            length += list[i].len;

        return 1;
    }

    return 0;
}

int find_timerange(char const* string,
                   int& startresult, int& finishresult,
                   int& start, int& length)
{
    Tokens list;
    parse_tokens(string, list);

    int index = 0;              // Index into "list"
    int pos = 0;                // Number of chars in "list[0..index-1]".
    while (index < list.size()) {
        int stseconds, fiseconds;
        int count = match_timerange(list, index, stseconds, fiseconds);
        if (count == 0) {
            // No time here, keep searching
            pos += list[index].len;
            index++;
            continue;
        }

        // Found time at "index"
        startresult = stseconds;
        finishresult = fiseconds;
        start = pos;
        length = 0;
        for (int i = index; i < index + count; i++)
            length += list[i].len;

        return 1;
    }

    return 0;
}
