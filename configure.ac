AC_INIT([ical],[3.0.5])
AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT

AC_CONFIG_MACRO_DIRS([m4])

# Create config files
AC_CONFIG_HEADERS([config.h])

############################################################################
# Check for programs

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PATH_PROGS([MAILER],[Mail mail],[Mail])
AC_PROG_CXX
# RANLIB needed for FreeBSD
AC_PROG_RANLIB
AC_PROG_SED

#### rpmversion passed in from the .spec file
AC_MSG_CHECKING(for rpmversion)
AC_ARG_WITH(rpmversion,
            [AS_HELP_STRING([--with-rpmversion=version],[version from rpm .spec file])],
            [rpmversion="$withval"],
            [rpmversion="$PACKAGE_VERSION"])
AC_MSG_RESULT($rpmversion)


############################################################################
# Find Tcl/Tk

AS_MESSAGE([checking Tcl/Tk installation...])
legal_tk_versions="9.0"
legal_tcl_versions="9.0"

#### Find tclsh
AC_ARG_WITH(tclsh,
            [AS_HELP_STRING([--with-tclsh=<path>],[full path name of tclsh])],
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
           [AS_HELP_STRING([--with-tclconfig=<path>],[directory that contains tclConfig.sh])],
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
            [AS_HELP_STRING([--with-tkconfig=<path>],[directory that contains tkConfig.sh])],
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


# Generate lists of all the files that need to be installed
generated='startup\|ps_header'
ICAL_TCLLIB_SCRIPTS=`echo $srcdir/tcllib/*.tcl`
ICAL_SCRIPTS=`echo $srcdir/*.tcl|sed -e "s/[[^ ]]*\($generated\)[[^ ]]*//g"`
ICAL_CONTRIB=`echo $srcdir/contrib/*`
ICAL_IMAGES=`echo $srcdir/images/*`
ICAL_ICON=`echo $srcdir/images/$PACKAGE.svg`
ICAL_LICENSE=COPYING
ICAL_DOC=`echo $srcdir/doc/*.html $srcdir/ChangeLog`
ICAL_RPMVERSION=$rpmversion
pkgdocdir=`echo $datadir/doc/$PACKAGE`
AC_SUBST(ICAL_TCLLIB_SCRIPTS)
AC_SUBST(ICAL_SCRIPTS)
AC_SUBST(ICAL_CONTRIB)
AC_SUBST(ICAL_IMAGES)
AC_SUBST(ICAL_ICON)
AC_SUBST(ICAL_LICENSE)
AC_SUBST(ICAL_DOC)
AC_SUBST(ICAL_RPMVERSION)
AC_SUBST(pkgdocdir)

AC_CHECK_FUNCS(fsync)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

