# Copyright (c) 1993 by Sanjay Ghemawat
#############################################################################
# Print calendar contents

proc psmonth {output date} {
    global month_name

    set m [date month $date]
    set y [date year $date]
    set start [date make 1 $m $y]
    set finish [date make [date monthsize $date] $m $y]

    # rows and columns
    puts $output "5 7"
    # left header
    puts $output "$month_name($m), $y"
    #right header
    ps_printtime $output

    set col [expr [date weekday $start] - 1]
    set row 0

    # column heading
    if [cal option MondayFirst] {
        puts $output "Mon,Tue,Wed,Thu,Fri,Sat,Sun"
        set col [expr ($col - 1) % 7]
    } else {
        puts $output "Sun,Mon,Tue,Wed,Thu,Fri,Sat"
    }

    set num 1
    for {set d $start} {$d <= $finish} {incr d} {
        # identify this cell
        puts $output "$row $col"
        # cell left header
        puts $output ""
        # cell right header
        puts $output "$num"

        ps_printday $output $d

        incr num
        incr col
        if {$col == 7} {
            set col 0
            incr row
            if {$row == 5} {set row 0}
        }
    }
}

proc psdays {output start num} {
    set wdays {{} Sun Mon Tue Wed Thu Fri Sat}
    set mons  {{} Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec}

    set finish [expr $start + $num - 1]
    set year1 [date year $start]
    set year2 [date year $finish]
    # Try to keep column width and row height similar
    set cols [expr round(sqrt($num * 11.0 / 8.5))]
    set rows [expr ($num + $cols - 1) / $cols]

    # rows and columns
    puts $output "$rows $cols"
    # left header
    if {$year1 != $year2} {
        puts $output "$year1 - $year2"
    } else {
        puts $output "$year1"
    }
    #right header
    ps_printtime $output

    set col 0
    set row 0

    # column heading
    puts $output ""

    for {set d $start} {$d <= $finish} {incr d} {
        # identify this cell
        puts $output "$row $col"
        # cell left header
        puts $output "[lindex $wdays [date weekday $d]]"
        # cell right header
        puts $output "[lindex $mons  [date month $d]] [date monthday $d]"

        ps_printday $output $d

        incr col
        if {$col == $cols} {
            set col 0
            incr row
        }
    }
}

#############################################################################
# Internal operations

proc ps_printtime {output} {
    # Get time/date
    set now [ical_time now]
    set date [date2text [date today]]
    set time [time2text [expr [ical_time minute $now] + [ical_time hour $now]*60]]

    # Get user
    set user ""
    catch {set user " by [exec whoami]"}

    puts $output "Printed $date, $time$user"
}

proc ps_printday {output date} {
    cal query $date $date item junk {
        # Print all the lines
        set text [item2text $date $item "" "" 2000]
        puts $output "77143db1-ce11-4ba8-b68e-497add32185e"
        puts $output $text
        puts $output "eadb44e9-3c78-44b0-a4f8-0604154b692f"
    }
}

