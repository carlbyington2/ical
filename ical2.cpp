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

std::map<std::string, Item*> items;     // uid -> item
Calendar* calendar = NULL;              // calendar we are working on

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

struct icaltimetype count_to_until(struct icalrecurrencetype recur, struct icaltimetype start) {
    icalrecur_iterator *ritr = icalrecur_iterator_new(recur, start);
    int count = recur.count;
    int index = 0;
    struct icaltimetype next;
    struct icaltimetype final_occurrence;
    next = icalrecur_iterator_next(ritr);
    while (!icaltime_is_null_time(next)) {
        index++;
        if (index == count) {
            final_occurrence = next;
            break;
        }
        next = icalrecur_iterator_next(ritr);
    }
    icalrecur_iterator_free(ritr);
    return final_occurrence;
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

std::string get_tzid(icalcomponent* comp, char *dtstart) {
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_DTSTART_PROPERTY);
    if (pr) {
        icalparameter *pa = icalproperty_get_first_parameter(pr, ICAL_TZID_PARAMETER);
        if (pa) return TranslateWindowsToIana(icalparameter_get_tzid(pa));
    }
    if (dtstart && (dtstart[strlen(dtstart)-1] == 'Z')) return "Etc/UTC";
    return "<Local>";
}

int get_duration(icalcomponent* comp) {
    // in minutes
    int rc = 0;
    struct icaldurationtype duration = icalcomponent_get_duration(comp);
    if (!icaldurationtype_is_null_duration(duration)) {
        rc = icaldurationtype_as_int(duration);
    }
    return rc / 60;
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

std::string get_weekdays(struct icalrecurrencetype recur, int& nth) {
    std::string wdays = "";
    const std::string day_names[] = {"", "1", "2", "3", "4", "5", "6", "7"};
    for (int i=0; (i<ICAL_BY_DAY_SIZE) && (recur.by_day[i] != ICAL_RECURRENCE_ARRAY_MAX); i++) {
        // Extract the day part by decoding the encoded short int
        icalrecurrencetype_weekday day = icalrecurrencetype_day_day_of_week(recur.by_day[i]);
        if (day >= 1 && day <= 7) {
            if (!wdays.empty()) {
                wdays += " ";
            }
            wdays += day_names[day];
            nth = icalrecurrencetype_day_position(recur.by_day[i]);
        }
    }
    return wdays;
}

void traverse_components(icalcomponent* comp, int depth) {
    if (!comp) return;

    icalcomponent_kind kind = icalcomponent_isa(comp);
    if ((kind == ICAL_VEVENT_COMPONENT) ||
        (kind == ICAL_VJOURNAL_COMPONENT) ||
        (kind == ICAL_VTODO_COMPONENT)) {
        const char *uid                 = icalcomponent_get_uid(comp);
        char *dtstamp                   = get_property(comp, ICAL_DTSTAMP_PROPERTY);
        const char *summary             = icalcomponent_get_summary(comp);
        char *dtstart                   = get_property(comp, ICAL_DTSTART_PROPERTY);
        std::string tzid                = get_tzid(comp, dtstart);
        struct icaltimetype dt          = icalcomponent_get_dtstart(comp);
        int   duration                  = get_duration(comp);
        bool  have_rrule;
        struct icalrecurrencetype recur = get_rrule(comp, have_rrule);
        enum icalproperty_status status = icalcomponent_get_status(comp);
        if (uid && dtstamp && summary && dtstart) {
            auto it = items.find(std::string(uid));
            if (it != items.end()) {
                // we already have an item matching uid
                Item* item = it->second();
                const char* lm = item->GetLastModified();
                if (strcmp(mod, dtstamp) >= 0) {
                    // our last modified is >= the dtstamp on the incoming item, ignore it
                    free(dtstamp);
                    free(dtstart);
                    return;
                } else {
                    // our last modified is < the dtstamp on the incoming item
                    // delete our old version
                    calendar->Remove(item);
                }
            }
        }
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
                char freq[100]{};
                if (recur.freq == ICAL_DAILY_RECURRENCE) {
                    snprintf(freq, sizeof(freq), "[Days %s %d\n", firstday, recur.interval);
                }
                if (recur.freq == ICAL_WEEKLY_RECURRENCE) {
                    int junk;
                    std::string wdays = get_weekdays(recur, junk);
                    snprintf(freq, sizeof(freq), "[Weekdays %s Months 1 2 3 4 5 6 7 8 9 10 11 12\n", wdays.c_str());
                }
                if (recur.freq == ICAL_MONTHLY_RECURRENCE) {
                    if ((recur.by_month_day[0] != 0) &&
                        (recur.by_month_day[0] != ICAL_RECURRENCE_ARRAY_MAX)) {
                        // there may be multiple by_monthday entries, but we can
                        // only represent one
                        snprintf(freq, sizeof(freq), "[Months %s %d\n", firstday, recur.interval);
                    }
                    if ((recur.by_set_pos[0] != 0) &&
                        (recur.by_set_pos[0] != ICAL_RECURRENCE_ARRAY_MAX)) {
                        // there may be multiple by_set_pos entries, but we can
                        // only represent one, and that must be positive
                        // first weekday of month
                        // ComplexMonths 1 1 1/10/2026 Forward ByWorkDay
                        // 6th weekday of month
                        // ComplexMonths 1 6 8/10/2026 Forward ByWorkDay
                        // 2nd thursday of month
                        // ComplexMonths 1 2 8/10/2026 Forward ByWeek 5
                        // 2nd thursday every 2 months
                        // ComplexMonths 2 2 8/10/2026 Forward ByWeek 5
                        // 4th-last thursday every 2 months
                        // ComplexMonths 2 4 8/10/2026 Backward ByWeek 5
                        int nth = recur.by_set_pos[0];
                        const char* direction;
                        if (nth > 0) {
                            direction = "Forward";
                        } else {
                            direction = "Backward";
                            nth = 0 - nth;
                        }
                        int junk;
                        std::string wdays = get_weekdays(recur, junk);
                        if (wdays == "2 3 4 5 6") {
                            // working day m-f
                            snprintf(freq, sizeof(freq), "[ComplexMonths %d %d %s %s ByWorkDay\n", recur.interval, nth, firstday, direction);
                        }
                        else if (wdays.length() == 1) {
                            // single day
                            snprintf(freq, sizeof(freq), "[ComplexMonths %d %d %s %s ByWeek %s\n", recur.interval, nth, firstday, direction, wdays.c_str());
                        }
                    }
                    if ((recur.by_day[0] != 0) &&
                        (recur.by_day[0] != ICAL_RECURRENCE_ARRAY_MAX)) {
                        // 2nd thursday every 2 months
                        // ComplexMonths 2 2 8/10/2026 Forward ByWeek 5
                        int nth = recur.by_day[0];
                        const char* direction;
                        if (nth > 0) {
                            direction = "Forward";
                        } else {
                            direction = "Backward";
                            nth = 0 - nth;
                        }
                        std::string wdays = get_weekdays(recur, nth);
                        if (wdays.length() == 1) {
                            // single day
                            snprintf(freq, sizeof(freq), "[ComplexMonths %d %d %s %s ByWeek %s\n", recur.interval, nth, firstday, direction, wdays.c_str());
                        }
                    }
                }
                if (strlen(freq) > 0) {
                    // have a known recurrence type
                    char start[20]{};
                    char finish[20]{};
                    snprintf(start, sizeof(start), "Start %s\n", firstday);
                    if (icaltime_is_null_time(recur.until) && (recur.count > 0))
                        recur.until = count_to_until(recur, dt);
                    if (!icaltime_is_null_time(recur.until)) {
                        snprintf(finish, sizeof(finish), "Finish %d/%d/%d\n", recur.until.day, recur.until.month, recur.until.year);
                    }
                    std::string exclude = get_exdate(comp);
                    snprintf(dates, sizeof(dates), "%s%s%s%s", freq, start, finish, exclude.c_str());
                }
                else {
                    // unknown recurrence type
                    snprintf(dates, sizeof(dates), "[Single %d/%d/%d\n", dt.day, dt.month, dt.year);
                }
            }
            else {
                // no recurrence rule
                snprintf(dates, sizeof(dates), "[Single %d/%d/%d\n", dt.day, dt.month, dt.year);
            }
            Item* item;
            if (duration > 0) {
                item = new Appointment;
                item->SetUid(uid);
                item->SetLastModified(dtstamp);
                item->SetText(summary);
                item->SeetTimezone(tzid.c_str(), false);
                item->SetStart(atoi(start));
                item->SetLength(atoi(length));
                output("\nAppt [\n");
                output("Uid", uid);
                output("LastModified", dtstamp);
                output("Contents", summary);
                output("Start", start);
                output("Length", length);
                output("Timezone", tzid.c_str());
                output("Dates", dates);
                output("End ]\n");
            } else {
                item = new Notice;
                item->SetUid(uid);
                item->SetLastModified(dtstamp);
                item->SetText(summary);
                output("\nNote [\n");
                output("Uid", uid);
                output("LastModified", dtstamp);
                output("Contents", summary);
                output("Dates", dates);
                output(" End ]\n");
            }
            if (kind == ICAL_VTODO_COMPONENT) {
                item->SetTodo(1);
                output("Todo", "");
                if (status == ICAL_STATUS_COMPLETED) {
                    item->SetDone(1);
                    output("Done", "");
                }
            }
            calendar->Add(item);
            output("]\n");
        }
        free(dtstamp);
        free(dtstart);
    }

    icalcomponent* child = icalcomponent_get_first_component(comp, ICAL_ANY_COMPONENT);
    while (child) {
        // Recursive call for depth-first traversal
        traverse_components(child, depth + 1);
        child = icalcomponent_get_next_component(comp, ICAL_ANY_COMPONENT);
    }
}

// "argv == <cmd> <cal>"
int Cmd_Import_ICS(ClientData, Tcl_Interp* tcl, int argc, const char* argv[]) {
    if (argc != 2) {
        TCL_Error(tcl, "illegal number of arguments");
    }

    Calendar_Tcl* cal = find_cal(tcl, argv[1]);
    if (cal == 0) {TCL_Error(tcl, "illegal calendar");}

    // create items map of uid -> item
    calendar = cal->main->GetCalendar();
    for (int i = 0; i < calendar->Size(); i++) {
        Item* item = calendar->Get(i);
        if (!item) continue;
        std::string uid = std::string(item->GetUid());
        if (items.find(uid) == items.end()) {
            // have not seen this uid, add it to the map
            items[uid] = item;
        }
    }

    char *line;
    icalcomponent *c;

    icalparser *parser = icalparser_new();
    icalparser_set_gen_data(parser, stdin);
    printf("%s", "Calendar [v3.0]");
    do {
        line = icalparser_get_line(parser, read_stream);
        c = icalparser_add_line(parser, line);
        if (c != 0) {
            traverse_components(c, 0);
            icalcomponent_free(c);
        }
    } while (line != 0);
    icalparser_free(parser);
}
