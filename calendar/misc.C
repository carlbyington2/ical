/* Copyright (c) 1994 Sanjay Ghemawat */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include "basic.h"
#include "arrays.h"
#include "misc.h"

char* copy_string(char const* str) {
    char* out = new char[strlen(str)+1];
    strcpy(out, str);
    return out;
}

char const* my_name() {
    static char const* name = 0;	// My user name
    static int got_name = 0;		// Has my_name been initialized?

    if (! got_name) {
	// Fetch my name from user database
	struct passwd* pw = getpwuid(getuid());
	if (pw != 0) name = copy_string(pw->pw_name);
	got_name = 1;
    }

    return name;
}

int copy_file(char const* src, char const* dst) {
    FILE* in = fopen(src, "r");
    if (in == 0) return 0;

    FILE* out = fopen(dst, "w");
    if (out == 0) {
	fclose(in);
	return 0;
    }

    static const int bufsize = 1024;
    char buf[bufsize];
    while (1) {
	int readcount = fread(buf, sizeof(char), bufsize, in);
	if (readcount <= 0) break;

	int writecount = fwrite(buf, sizeof(char), readcount, out);
	if (readcount != writecount) break;
    }

    if (ferror(in) || ferror(out)) {
	fclose(in);
	fclose(out);
	return 0;
    }

    fclose(in);
    if (fclose(out) < 0) return 0;

    return 1;
}

void append_string(charArray* cbuf, char const* str) {
    cbuf->concat(str, strlen(str));
}

// XXX This is a big hack...
void format(charArray* cbuf, char const* format ...) {
    // XXX Need fixed size limit because vsprintf is broken
    static char buffer[4096];

    va_list ap;
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    append_string(cbuf, buffer);
}
