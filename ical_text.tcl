# Copyright (c) 1993 by Sanjay Ghemawat
#
# Startup script for text-based ical

proc ical_no_tk_script {} {
    # Parse arguments (some argument parsing has already been
    # done by startup.tcl)
    global argv ical

    set showcount 1
    set doaddone  0
    set doaddmul  0
    set doexport  0
    set doprint   0
    set dolist    0

    while {[llength $argv] != 0} {
        set arg [lindex $argv 0]
        set argv [lrange $argv 1 end]

        switch -- $arg {
            "-addmultiple" {
                set doaddmul 1
            }
            "-addone" {
                set doaddone 1
            }
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

    if $doaddmul {
        # Adapted from "icaladd" by "ark@research.att.com".
        puts stderr =====
        while {[gets stdin line] >= 0} {
            set item [item_parse $line]
            puts stderr [date2text [$item first]]
            puts -nonewline stderr [item2text notify $item]
            cal add $item
            puts stderr =====
        }
        cal save
    }

    if $doaddone {
        set text {}
        while {[gets stdin line] >= 0} {
            lappend text $line
        }
        set text [join $text "\n"]

        set item [item_parse $text]
        cal add $item
        cal save
        puts stderr =====
        puts stderr [date2text [$item first]]
        puts -nonewline stderr [item2text notify $item]
        puts stderr =====
    }

    if $doexport {
        export_ics cal
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
    }

    cal delete
    exit 0
}
