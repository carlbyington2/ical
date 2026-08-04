# Copyright (c) 1996  by Sanjay Ghemawat
# Indicate time of day in appointment view

append-hook dayview-startup {view} {
    global ical
    set scale  $ical(dpi_scaling)
    set m2 [expr {2 * $scale}]
    set m4 [expr {4 * $scale}]

    set c [$view window].al.c
    $c create line -1 -1 -1 -1 -arrow last \
        -arrowshape "${m2}m ${m4}m ${m2}m" \
        -fill [pref apptLineColor] -tags {tod}

    position_tod [$view appt_list] $c
}

# Position the time-of-day indicator in the specified canvas "c".
# Also set up a call back to this procedure at the next minute boundary.
proc position_tod {al c} {
    if ![winfo exists $c] {return}

    global ical
    set scale  $ical(dpi_scaling)
    set m2 [expr {2 * $scale}]
    set m4 [expr {4 * $scale}]

    set t [ical_time split [ical_time now]]
    set m [expr [lindex $t 0]*60 + [lindex $t 1]]
    set y [expr ($m * [$al line_height]) / 30]
    $c coords tod "${m2}m $y ${m4}m $y"

    # Schedule next firing (at minute boundary)
    set msec [expr (60 - [ical_time second [ical_time now]])*1000]
    after $msec position_tod $al $c
}
