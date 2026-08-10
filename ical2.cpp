#include <libical/ical.h>

#include <assert.h>
#include <stdlib.h>
#include <cstring>

char *read_stream(char *s, size_t size, void *d)
{
    return fgets(s, (int)size, (FILE *)d);
}

char *get_property(icalcomponent* comp, icalproperty_kind prop) {
    char *rc = NULL;
    icalproperty *pr = icalcomponent_get_first_property(comp, prop);
    if (pr) rc = icalproperty_as_ical_string_r(pr);
    return rc;
}

char *get_parameter(icalcomponent* comp, icalproperty_kind prop, icalparameter_kind parm) {
    char *rc = NULL;
    icalproperty *pr = icalcomponent_get_first_property(comp, prop);
    if (pr) {
        icalparameter *pa = icalproperty_get_first_parameter(pr, parm);
        if (pa) {
            rc = icalparameter_as_ical_string_r(pa);
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
    struct icaltimetype rc {0};
    icalproperty *pr = icalcomponent_get_first_property(comp, ICAL_DTSTART_PROPERTY);
    if (pr) {
        rc = icalproperty_get_dtstart(pr);
    }
    return rc;
}

void output(const char *key, const char *value) {
    printf("%s [%s]", key, value);
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
        char *uid              = get_property(comp, ICAL_UID_PROPERTY);
        char *dtstamp          = get_property(comp, ICAL_DTSTAMP_PROPERTY);
        char *summary          = get_property(comp, ICAL_SUMMARY_PROPERTY);
        char *dtstart          = get_property(comp, ICAL_DTSTART_PROPERTY);
        char *tzid             = get_parameter(comp, ICAL_DTSTART_PROPERTY, ICAL_TZID_PARAMETER);
        struct icaltimetype dt = get_dtstart(comp);
        int   duration         = get_duration(comp);
        char *rrule            = get_property(comp, ICAL_RRULE_PROPERTY);
        char *status           = get_property(comp, ICAL_STATUS_PROPERTY);
        if (uid && dtstamp && summary && dtstart && !rrule) {   // we cannot handle RRULE yet
            char start[10];
            snprintf(start, sizeof(start), "%d", round(dt.hour*60 + dt.minute,15));
            char length[10];
            snprintf(length, sizeof(length), "%d", round(duration,15));
            char date[20];
            snprintf(date, sizeof(date), "%d/%d/%d", dt.day, dt.month, dt.year);
            if (duration > 0) {
                output("Appt [\n");
                output("Uid", uid);
                output("LastModified", dtstamp);
                output("Contents", summary);
                output("Start", start);
                output("Length", length);
                if (tzid) output("Timezone", tzid);
                output("Dates [Single ");
                output(date);
                output(" End ]\n");
            } else {
                output("Note [\n");
                output("Uid", uid);
                output("LastModified", dtstamp);
                output("Contents", summary);
                output("Start", start);
                output("Dates [Single ");
                output(date);
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
        free(summary );
        free(dtstart );
        free(tzid    );
        free(rrule   );
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
