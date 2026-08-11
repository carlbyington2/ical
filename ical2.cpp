#include <libical/ical.h>

#include <assert.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <unicode/ucal.h>
#include <unicode/ustring.h>
#include <unicode/timezone.h>
#include <unicode/unistr.h>
#include <iostream>
#include <string>

std::string TranslateWindowsToIana(const char *windowsZoneName) {
    // test if already an iana zone name
    if (strstr(windowsZoneName, "/")) return std::string(windowsZoneName);

    // no, convert to iana
    icu::UnicodeString winZone(windowsZoneName);
    icu::UnicodeString ianaZone;
    UErrorCode status = U_ZERO_ERROR;

    // Query CLDR mapping via ICU
    // The second parameter specifies the region filter.
    // Passing "001" or an empty string returns the primary/default IANA zone.
    icu::TimeZone::getIDForWindowsID(winZone, "", ianaZone, status);

    std::string ianaStr;
    if (U_SUCCESS(status) && !ianaZone.isEmpty()) {
        ianaZone.toUTF8String(ianaStr);
    }
    return ianaStr;
}


char *read_stream(char *s, size_t size, void *d)
{
    return fgets(s, (int)size, (FILE *)d);
}

char *get_property(icalcomponent* comp, icalproperty_kind prop) {
    char *rc = NULL;
    icalproperty *pr = icalcomponent_get_first_property(comp, prop);
    if (pr) rc = icalproperty_get_value_as_string_r(pr);
    return rc;
}

const char *get_summary(icalcomponent* comp) {
    const char *rc = NULL;
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_SUMMARY_PROPERTY);
    if (pr) rc = icalproperty_get_summary(pr);
    return rc;
}

const char *get_tzid(icalcomponent* comp) {
    const char *rc = NULL;
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_DTSTART_PROPERTY);
    if (pr) {
        icalparameter *pa = icalproperty_get_first_parameter(pr, ICAL_TZID_PARAMETER);
        if (pa) {
            rc = icalparameter_get_tzid(pa);
        }
    }
    return rc;
}

int get_duration(icalcomponent* comp) {
    // in minutes
    int rc = -1;
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_DURATION_PROPERTY);
    if (pr) {
        struct icaldurationtype duration = icalproperty_get_duration(pr);
        rc = icaldurationtype_as_int(duration);
    }
    return rc / 60;
}

struct icaltimetype get_dtstart(icalcomponent* comp) {
    struct icaltimetype rc{};
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_DTSTART_PROPERTY);
    if (pr) {
        rc = icalproperty_get_dtstart(pr);
    }
    return rc;
}

struct icalrecurrencetype get_rrule(icalcomponent* comp, bool &have_rrule) {
    have_rrule = false;
    struct icalrecurrencetype rc{};
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_RRULE_PROPERTY);
    if (pr) {
        rc = icalproperty_get_rrule(pr);
        have_rrule = true;
    }
    return rc;
}

std::string get_exdate(icalcomponent* comp) {
    std::string rc;
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_EXDATE_PROPERTY);
    while (pr) {
        struct icaltimetype exdate = icalproperty_get_exdate(pr);
        char d[40];
        snprintf(d, sizeof(d), "Deleted %d/%d/%d\n", exdate.day, exdate.month, exdate.year);
        std::string dd(d);
        rc.append(dd);
        pr = icalcomponent_get_next_property(comp, ICAL_EXDATE_PROPERTY);
    }
    return rc;
}

void output(const char *key, const char *value) {
    if (value[0] == '[') printf("%s %s", key, value);
    else                 printf("%s [%s]\n", key, value);
}

void output(const char *key) {
    printf("%s", key);
}

int round(int v, int d) {
    int half = d / 2;
    return ((v + half) / d) * d;
}

void traverse_components(icalcomponent* comp, int depth) {
    if (!comp) return;

    icalcomponent_kind kind = icalcomponent_isa(comp);
    if ((kind == ICAL_VEVENT_COMPONENT) ||
        (kind == ICAL_VJOURNAL_COMPONENT) ||
        (kind == ICAL_VTODO_COMPONENT)) {
        char *uid                       = get_property(comp, ICAL_UID_PROPERTY);
        char *dtstamp                   = get_property(comp, ICAL_DTSTAMP_PROPERTY);
        const char *summary             = get_summary(comp);
        char *dtstart                   = get_property(comp, ICAL_DTSTART_PROPERTY);
        const char *tzid                = get_tzid(comp);
        struct icaltimetype dt          = get_dtstart(comp);
        int   duration                  = get_duration(comp);
        bool  have_rrule;
        struct icalrecurrencetype recur = get_rrule(comp, have_rrule);
        char *status                    = get_property(comp, ICAL_STATUS_PROPERTY);
        if (uid && dtstamp && summary && dtstart) {
            // build start from dtstart time
            char start[20];
            snprintf(start, sizeof(start), "%d", round(dt.hour*60 + dt.minute,15));
            // build firstday from dtstart date
            char firstday[20];
            snprintf(firstday, sizeof(firstday), "%d/%d/%d", dt.day, dt.month, dt.year);
            // build length from duration
            char length[20];
            snprintf(length, sizeof(length), "%d", round(duration,15));
            // build dates from dtstart date and rrule
            char dates[220];
            if (have_rrule &&
                ((recur.freq == ICAL_DAILY_RECURRENCE) ||
                 (recur.freq == ICAL_WEEKLY_RECURRENCE) ||
                 (recur.freq == ICAL_MONTHLY_RECURRENCE))) {
                char fkey[20];
                strcpy(fkey, "Days");
                if (recur.freq == ICAL_MONTHLY_RECURRENCE) strcpy(fkey, "Months");
                char freq[100]{};
                char start[20]{};
                char finish[20]{};
                if (recur.freq == ICAL_WEEKLY_RECURRENCE) {
                    const std::string day_names[] = {"", "1", "2", "3", "4", "5", "6", "7"};
                    std::string wdays = "";
                    for (int i = 0; i < ICAL_BY_DAY_SIZE && recur.by_day[i] != ICAL_RECURRENCE_ARRAY_MAX; i++) {
                        // Extract the day part by decoding the encoded short int
                        icalrecurrencetype_weekday day = icalrecurrencetype_day_day_of_week(recur.by_day[i]);
                        if (day >= 1 && day <= 7) {
                            if (!wdays.empty()) {
                                wdays += " ";
                            }
                            wdays += day_names[day];
                        }
                    }
                    snprintf(freq, sizeof(freq), "[Weekdays %s Months 1 2 3 4 5 6 7 8 9 10 11 12\n", wdays.c_str());
                }
                else {
                    snprintf(freq, sizeof(freq), "[%s %s %d\n", fkey, firstday, recur.interval);
                }
                snprintf(start, sizeof(start), "Start %s\n", firstday);
                if (!icaltime_is_null_time(recur.until)) {
                    snprintf(finish, sizeof(finish), "Finish %d/%d/%d\n", recur.until.day, recur.until.month, recur.until.year);
                }
                std::string exclude = get_exdate(comp);
                snprintf(dates, sizeof(dates), "%s%s%s%s", freq, start, finish, exclude.c_str());
            }
            else {
                snprintf(dates, sizeof(dates), "[Single %d/%d/%d\n", dt.day, dt.month, dt.year);
            }
            if (duration > 0) {
                output("Appt [\n");
                output("Uid", uid);
                output("LastModified", dtstamp);
                output("Contents", summary);
                output("Start", start);
                output("Length", length);
                if (tzid) output("Timezone", tzid);
                output("Dates", dates);
                output("End ]\n");
            } else {
                output("Note [\n");
                output("Uid", uid);
                output("LastModified", dtstamp);
                output("Contents", summary);
                output("Start", start);
                output("Dates", dates);
                output(" End ]\n");
            }
            if (kind == ICAL_VTODO_COMPONENT) {
                output("Todo", "");
                if (status && (strcmp(status, "COMPLETED") == 0))
                    output("Done", "");
            }
            output("]\n");
        }
        free(uid     );
        free(dtstamp );
        free(dtstart );
        free(status  );
    }

    icalcomponent* child = icalcomponent_get_first_component(comp, ICAL_ANY_COMPONENT);
    while (child) {
        // Recursive call for depth-first traversal
        traverse_components(child, depth + 1);
        child = icalcomponent_get_next_component(comp, ICAL_ANY_COMPONENT);
    }
}

int main(int argc, char *argv[])
{
    char *line;
    FILE *stream;
    icalcomponent *c;

    /* Create a new parser object */
    icalparser *parser = icalparser_new();

    stream = fopen(argv[1], "r");

    assert(stream != 0);

    /* Tell the parser what input route it should use. */
    icalparser_set_gen_data(parser, stream);

    do {
        /* Get a single content line by making one or more calls to
           read_stream()*/
        line = icalparser_get_line(parser, read_stream);

        /* Now, add that line into the parser object. If that line
           completes a component, c will be non-zero */
        c = icalparser_add_line(parser, line);

        if (c != 0) {
            traverse_components(c, 0);
            //char *temp = icalcomponent_as_ical_string_r(c);
            //printf("%s", temp);
            //free(temp);
            //printf("\n---------------\n");
            icalcomponent_free(c);
        }

    } while (line != 0);

    icalparser_free(parser);
}
