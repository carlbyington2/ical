/* Copyright (c) 1996  by Sanjay Ghemawat */
#include <sys/param.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "basic.h"
#include "config.h"
#include "misc.h"
#include "uid.h"

#ifndef HAVE_GETHOSTNAME_PROTO

#ifdef linux
extern "C" int gethostname(char*, unsigned int);
#else /* !linux */
extern "C" int gethostname(char*, int);
#endif /* linux */

#endif

#ifdef HAVE_GETHOSTID
#ifndef HAVE_GETHOSTID_PROTO
extern "C" long gethostid(void);
#endif
#endif

#ifndef HAVE_GETPID_PROTO
extern "C" pid_t getpid(void);
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 1000
#endif

char const* uid_new() {
    static int   inited = 0;
    static char  hostname[MAXHOSTNAMELEN+1];
    static char  buffer[MAXHOSTNAMELEN+1000];
    static int   hostid = 0;
    static pid_t pid = 0;
    static int   counter = 0;

    if (! inited) {
	if (gethostname(hostname, MAXHOSTNAMELEN+1) < 0) {
	    // Error?
	    strcpy(hostname, "unknownhost");
	}
#ifdef HAVE_GETHOSTID
	hostid = gethostid();
#else
	hostid = 0;
#endif
	pid = getpid();
	counter = 0;
	inited = 1;
    }

    sprintf(buffer, "%s_%x_%x_%x", hostname, hostid, pid, counter);
    counter++;

    return copy_string(buffer);
}

implementOpenHashSet(UidSet,char const*,hash_string,cmp_string)
