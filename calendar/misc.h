/* Copyright (c) 1994 Sanjay Ghemawat */
#ifndef _MISC_H
#define _MISC_H

/* Support routines. */

extern char* copy_string(char const* str);
// effects - Return a newly allocated copy of "str".

extern char const* my_name();
    // effects - Return user login name.  Returns 0 on error.

extern int copy_file(char const* src, char const* dst);
    // effects - Copy file named by "src" to file named by "dst"
    //           Return true iff successful.

class charArray;

extern void append_string(charArray*, char const* str);
    // effects - Append string to end of a buffer.  No null character
    //           is ever appended or removed.

extern void format(charArray*, char const* format, ...);
    // effects - sprintf to the end of a buffer.  No null character
    //           is ever appended or removed.

#endif /* _MISC_H */
