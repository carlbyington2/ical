/* Copyright unknown? */
#ifndef _STATFIX_H
#define _STATFIX_H

#include <sys/types.h>
#include <sys/stat.h>

/* Definition of "S_ISLNK" stolen from Perl v5.0. */

/* The stat macros for Amdahl UTS, Unisoft System V/88 (and derivatives
   like UTekV) are broken, sometimes giving false positives.  Undefine
   them here and let the code below set them to proper values.

   The ghs macro stands for GreenHills Software C-1.8.5 which
   is the C compiler for sysV88 and the various derivatives.
   This header file bug is corrected in gcc-2.5.8 and later versions.
   --Kaveh Ghazi (ghazi@noc.rutgers.edu) 10/3/94.  */

#if defined(uts) || (defined(m88k) && defined(ghs))
#   undef S_ISLNK
#endif

#ifndef S_ISLNK
#   ifdef _S_ISLNK
#       define S_ISLNK(m) _S_ISLNK(m)
#   else
#       ifdef _S_IFLNK
#           define S_ISLNK(m) ((m & S_IFMT) == _S_IFLNK)
#       else
#           ifdef S_IFLNK
#               define S_ISLNK(m) ((m & S_IFMT) == S_IFLNK)
#           else
#               define S_ISLNK(m) (0)
#           endif
#       endif
#   endif
#endif

#endif /* _STATFIX_H */
