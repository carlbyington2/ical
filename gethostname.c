/* Copyright (c) 1994 by Sanjay Ghemawat */
/* Definition of "gethostname" for SYSV and Posix.1 systems. */

#include "config.h"

#if defined(HAVE_STRING_H)
#include <string.h>
#else
#include <strings.h>
#endif

#if defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
#include <sys/utsname.h>
#endif

int gethostname(name, len)
    char* name;
    int   len;
{
#if defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
    struct utsname uinfo;

    if (uname(&uinfo) < 0)
	return -1;

    strncpy(name, uinfo.nodename, len);
    return 0;
#else
    /* XXX Set "errno" */
    return -1;
#endif
}
