AC_INIT([ical], [3.0.4])
AM_INIT_AUTOMAKE([subdir-objects])

# Create config files
AC_CONFIG_HEADER(config.h)

############################################################################
# Check for programs

AC_PROG_INSTALL
AC_PROG_RANLIB
AC_LN_S
AC_PROGRAMS_PATH(MAILER,Mail mail,Mail)
AC_PROG_CXX
AC_PROG_SED

############################################################################
# Find Tcl/Tk

AC_CHECKING(Tcl/Tk installation)
legal_tk_versions="8.6 8.5 8.4 8.3 8.2 8.0 4.9 4.8 4.7 4.6 4.5 4.4 4.3 4.2 4.1"
legal_tcl_versions="8.6 8.5 8.4 8.3 8.2 8.0 7.9 7.8 7.7 7.6 7.5"

#### Find tclsh
AC_ARG_WITH(tclsh,
            [AC_HELP_STRING([--with-tclsh=<path>],[full path name of tclsh])],
            [tclsh="$withval"])
if test -n "$tclsh"; then
    AC_MSG_CHECKING(for tclsh)
    AC_MSG_RESULT($tclsh)
else
    tclsh_list=
    for v in $legal_tcl_versions; do
	tclsh_list="$tclsh_list tclsh$v"
    done
    # Look for tclsh variants in $prefix/bin, and then in $PATH.
    AC_PATH_PROGS(tclsh,$tclsh_list)
fi
if test -z "$tclsh"; then
    AC_MSG_ERROR([I could not find tclsh in your search PATH.

Please supply the location of a tclsh executable by
running configure with the following option
	--with-tclsh=<full path name of tclsh>])
fi

#### Run tclsh to find tclConfig.sh
AC_MSG_CHECKING(for tclConfig.sh)
AC_ARG_WITH(tclconfig,
           [AC_HELP_STRING([--with-tclconfig=<path>],
                           [directory that contains tclConfig.sh])],
           [tclconfig="$withval"])
if test -z "$tclconfig"; then
  if test -e /System/Library/Frameworks; then
    # Mac OS X
    tclconfig=/System/Library/Frameworks/Tcl.framework
  else
    [tclconfig=`echo 'puts [file dirname [info library]]' | $tclsh`]
    if test ! -f "$tclconfig/tclConfig.sh"; then
      # Debian, is that you?
      [tclconfig=`echo 'puts [info library]' | $tclsh`]
    fi
  fi
fi
if test ! -f "$tclconfig/tclConfig.sh"; then
    AC_MSG_ERROR([I could not find tclConfig.sh in $tclconfig.

Please specify the directory that contains tclConfig.sh
by running configure with the following option
	--with-tclconfig=<path name of directory containing tclConfig.sh>

Warning: Only Tcl versions 7.5 and later provide a tclConfig.sh file.
Make sure you have a new enough version installed on your
system.])
fi
AC_MSG_RESULT($tclconfig/tclConfig.sh)

#### Look for tkConfig.sh (in same directory as tclConfig.sh?)
AC_MSG_CHECKING(for tkConfig.sh)
AC_ARG_WITH(tkconfig,
            [AC_HELP_STRING([--with-tkconfig=<path>],
                            [directory that contains tkConfig.sh])],
            [tkconfig="$withval"])
if test -z "$tkconfig"; then
  if test -e /System/Library/Frameworks; then
    # Mac OS X
    tkconfig=/System/Library/Frameworks/Tk.framework
  else
    tkconfig="$tclconfig"
    if test ! -f "$tkconfig/tkConfig.sh"; then
      # Debian, is that you?
      [tkconfig=`echo 'puts [file join [file dirname [info library]] tk[info tclversion]]' | $tclsh`]
    fi
  fi
fi
if test ! -f "$tkconfig/tkConfig.sh"; then
    AC_MSG_ERROR([I could not find tkConfig.sh in $tkconfig.

Please specify the directory that contains tkConfig.sh
by running configure with the following option
	--with-tkconfig=<path name of directory containing tkConfig.sh>

Warning: Only Tk versions 4.1 and later provide a tkConfig.sh file.
Make sure you have a new enough version installed on your system.])
fi
AC_MSG_RESULT($tkconfig/tkConfig.sh)

#### Load the Tcl/Tk configuration files 
. $tclconfig/tclConfig.sh
. $tkconfig/tkConfig.sh

#### Export variables discovered during Tcl/Tk configuration

AC_SUBST(TCL_LIB_SPEC)
AC_SUBST(TCL_PREFIX)
AC_SUBST(TCL_EXEC_PREFIX)
AC_SUBST(TCL_LIBS)
AC_SUBST(TK_LIB_SPEC)
AC_SUBST(TK_LIBS)
AC_SUBST(TK_PREFIX)
AC_SUBST(TK_EXEC_PREFIX)
AC_SUBST(TK_XINCLUDES)
AC_SUBST(TCL_INCLUDE_SPEC)
AC_SUBST(TK_INCLUDE_SPEC)

# Generate list of all .tcl files that need to be installed
generated='startup\|ps_header'
ICAL_TCLLIB_SCRIPTS=`echo $srcdir/tcllib/*.tcl`
ICAL_SCRIPTS=`echo $srcdir/*.tcl|sed -e "s/[[^ ]]*\($generated\)[[^ ]]*//g"`
ICAL_CONTRIB=`echo $srcdir/contrib/*`

AC_SUBST(ICAL_TCLLIB_SCRIPTS)
AC_SUBST(ICAL_SCRIPTS)
AC_SUBST(ICAL_CONTRIB)

AC_CHECK_FUNCS(fsync)

AC_OUTPUT(Makefile)

