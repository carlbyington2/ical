# Copyright (c) 1993 by Sanjay Ghemawat
#
# Startup script for text-based ical

proc ical_no_tk_script {} {
    # Parse arguments (some argument parsing has already been
    # done by startup.tcl)
    global argv ical

    set showcount 1
    set doexport  0
    set doprint   0
    set dolist    0

    while {[llength $argv] != 0} {
        set arg [lindex $argv 0]
        set argv [lrange $argv 1 end]

        switch -- $arg {
            "-exportics" {
                set doexport 1
            }
            "-print" {
                if {[llength $argv] < 1} ical_usage
                set spec [lindex $argv 0]
                set argv [lrange $argv 1 end]

                # Check on format of show spec
                set showcount $spec
                set doprint 1
            }
            "-show" {
                if {[llength $argv] < 1} ical_usage
                set spec [lindex $argv 0]
                set argv [lrange $argv 1 end]

                # Check on format of show spec
                if ![regexp {^\+([0-9]+)$} $spec junk days] ical_usage
                set showcount $days
                set dolist 1
            }
            "-list" {
                set showcount 1
                set dolist 1
            }
            default {ical_usage}
        }
    }

    # Get calendar
    calendar cal $ical(calendar)

    if $doexport {
        export_ics
    }

    if $doprint {
        # Generate postscript
        set papersize SetUSLetter
        catch {set papersize [cal option PrintPaperSize]}
        fconfigure stdout -encoding iso8859-1
        puts stdout [pr_output $ical(startdate) $showcount $papersize]
    }

    if $dolist {
        # Generate listing
        set lastdate ""
        set sep ""
        cal listing $ical(startdate) [expr $ical(startdate)+$showcount-1] i d {
            if {$d != $lastdate} {
                puts stdout "$sep[date2text $d]"
                set lastdate $d
                set sep "\n"
            }
            puts -nonewline stdout [item2text $d $i]
        }
        cal delete
    }
    exit 0
}
