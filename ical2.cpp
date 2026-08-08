#include <libical/ical.h>

#include <assert.h>
#include <stdlib.h>

char *read_stream(char *s, size_t size, void *d)
{
    return fgets(s, (int)size, (FILE *)d);
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
            char *temp = icalcomponent_as_ical_string_r(c);
            printf("%s", temp);
            free(temp);

            printf("\n---------------\n");

            icalcomponent_free(c);
        }

    } while (line != 0);

    icalparser_free(parser);
}
