# Copyright (C) 2006 Ethan Blanton <elb@elitists.net>
# Version 0.9.3
#
# This program spits out an iCalendar format calendar of the ical
# calendar it loads.  As you can see, it's not at all finished;
# However, it's finished enough to be useful to me, so here it is.
#

proc preamble {c} {
    fconfigure $c -translation crlf
    puts $c "BEGIN:VCALENDAR"
    puts $c "VERSION:2.0"
    puts $c "PRODID:-//elitists.net//NONSGML ical2ics 1.0//EN"
}

proc postamble {c} {
    puts $c "END:VCALENDAR"
}

proc vtimezone {c tz} {
    puts $c "BEGIN:VTIMEZONE"
    puts $c "TZID:$tz"
    dstinfo $c $tz
    puts $c "END:VTIMEZONE"
}

proc iteminfo {c i d} {
    if {![info exists ::ic2ics_events]} {
        set ::ic2ics_events([$i uid]) 1
    } else {
        if {[info exists ::ic2ics_events([$i uid])]} {
            return
        } else {
            set ::ic2ics_events([$i uid]) 1
        }
    }
    if {[$i text] eq "" } return
    if {[$i is appt]} {
        vevent $c $i $d
    } elseif {[$i is note]} {
        vjournal $c $i $d
    }
}

proc vevent_or_journal {c i d item_d_or_t zone} {
    puts $c "UID:[$i uid]"
    puts $c "DTSTAMP:[$i last_modified]"
    puts $c "[text_fold SUMMARY:[string map [list "\n" {\n} "," {\,}] [$i text]]]"
    if {[$i repeats]} {
        set r [$i describe_repeat -terse]
        rrule $c $i $r $item_d_or_t $zone
        puts $c "DTSTART$zone:[$item_d_or_t $i [lindex $r 0]]"
    } else {
        puts $c "DTSTART$zone:[$item_d_or_t $i $d]"
    }
}

proc vevent {c i d} {
    puts $c "BEGIN:VEVENT"
    # get the time zone
    set z [$i timezone]
    if {[string length $z] > 0} {
        if {$z eq "<Local>"} {
            set z ""
        } else {
            set z ";TZID=$z"
        }
    }
    vevent_or_journal $c $i $d "item_time" $z
    puts $c "DURATION:[item_duration $i]"
    puts $c "END:VEVENT"
}

proc vjournal {c i d} {
    puts $c "BEGIN:VJOURNAL"
    vevent_or_journal $c $i $d "item_date" ""
    puts $c "END:VJOURNAL"
}

proc rrule {c i r item_d_or_t zone} {
    set rule ""
    switch -- [lindex $r 3] {
        "weekly" {
            set rule "FREQ=WEEKLY;BYDAY="
            set days [list]
            foreach d [lindex $r 4] { lappend days [i2d $d] }
            set rule "$rule[join $days ,]"
        }
        "daily" {
            set rule "FREQ=DAILY;INTERVAL=[lindex $r 4]"
        }
        "monthly" {
            set rule "FREQ=MONTHLY;INTERVAL=[lindex $r 4]"
            switch -- [lindex $r 5] {
                "day" {
                    set rule "$rule;BYMONTHDAY=[lindex $r 6]"
                }
                "week" {
                    set plus ""
                    if {[lindex $r 6] > 0} { set plus "+"}
                    set rule "$rule;BYDAY=$plus[lindex $r 6][i2d [lindex $r 7]]"
                }
                "workday" {
                    set rule "$rule;BYDAY=MO,TU,WE,TH,FR;BYSETPOS=[lindex $r 6]"
                }
            }
        }
        "monthset" {
            # I don't see a way to create one of these in ical, so I
            # don't yet support them.
        }
    }
    if {[lindex $r 1] != [date last]} {
        set rule "$rule;UNTIL=[$item_d_or_t $i [lindex $r 1]]"
    }
    puts $c "RRULE:$rule"
    if {[llength [lindex $r 2]]} {
        set l [list]
        foreach d [lindex $r 2] { lappend l [$item_d_or_t $i $d] }
        puts $c "EXDATE$zone:[join $l ,]"
    }
}

proc i2d {i} {
    lindex {SU MO TU WE TH FR SA} [expr {$i - 1}]
}

proc item_date {i d} {
    set d [format "%4d%02d%02d" [date year $d] [date month $d] \
                  [date monthday $d]]
}

proc item_time {i d} {
    set t [$i starttime native]
    set d [format "%sT%02d%02d00" [item_date $i $d] [expr {($t / 60) % 24}] \
		  [expr {$t % 60}]]
}

proc item_duration {i} {
    set d [$i length]
    set d "PT[expr {$d / 60}]H[expr {$d % 60}]M"
}

proc text_fold {t} {
    set x 0
    set i [expr {[string length $t] / 75}]
    if {$i == 0} {return $t}
    set t2 "[string range $t 0 74]"
    while {$x < $i} {
        incr x
	set t2 "$t2\n [string range $t [expr {$x * 74 + 1}] [expr {$x * 74 + 74}]]"
    }
    return $t2
}

proc export_ics {cal} {
    preamble stdout
    cal query [date first] [date last] i d {
        iteminfo stdout $i $d
    }
    postamble stdout
}
