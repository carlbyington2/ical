# Copyright (c) 1996  by Sanjay Ghemawat
# Temporary prototype for editing item properties

proc item_edit {leader item} {
    global iedit
    set iedit(done) -1

    iedit_make
    iedit_fill $item
    update
    wm minsize .iedit [winfo reqwidth .iedit] [winfo reqheight .iedit]

    dialog_run $leader .iedit iedit(done)
    if !$iedit(done) {return}

    # Check that item has not been deleted concurrentlya
    catch {
	iedit_save
    }
}

proc iedit_make {} {
    set f .iedit
    global iedit

    if [winfo exists $f] {return}

    toplevel $f -class Bigdialog
    wm title $f {Item Properties}
    wm iconname $f Item
    wm protocol $f WM_DELETE_WINDOW {set iedit(done) 0}

    # Make the various container frames
    frame $f.f1
    frame $f.f2
    frame $f.fa -class Pane

    # Make the buttons
    make_buttons $f.bot 1 {
	{Cancel		{set iedit(done) 0}}
	{Okay		{set iedit(done) 1}}
    }

    # Top-level layout
    pack $f.f1 -side top -fill both -expand 1
    pack $f.f2 -side top -fill both -expand 1
    pack $f.fa -side top -fill both -expand 1
    pack $f.bot -side top -fill x

    # Alarm help
    message $f.ahelp -aspect 500 -text [join {
	{Select set of alarm times in minutes.}
	{Create an alarm by dragging a marker out of the well at the}
	{right of the scale.}
	{You can also drag existing markers to change alarm times.}
	{If you drag a marker far enough up or down so that it turns}
	{dim, it will be deleted when you release the mouse button.}
    }]
    pack $f.ahelp -in $f.fa -side top -expand 1 -fill both

    # Make alarm ruler
    ruler $f.alarms {Alarms (in minutes)} 0 60 5 1 2.2m
    pack $f.alarms -in $f.fa -side left -padx 2m -pady 1m

    # Make top-frame subdivisions
    frame $f.f2.c1 -class Pane
    frame $f.f2.c2 -class Pane
    frame $f.f2.c3

    # Start/end times
    frame $f.times -class Pane
    pack $f.times -in $f.f1\
	-side right -fill both -expand 1 -ipadx 2m -ipady 1m

    iedit_make_editor $f.start   "Start Time"  iedit_change_start
    iedit_make_editor $f.finish  "Finish Time" iedit_change_finish
    pack $f.start  -in $f.times -side top -expand 1
    pack $f.finish -in $f.times -side top -expand 1

    # Make text
    text $f.text -relief raised -bd 1 -width 30 -height 3 -wrap word\
	-highlightthickness 0
    pack $f.text  -in $f.f1 -side left -fill both -expand 1 -ipadx 1m -ipady 1m

    pack $f.f2.c3 -side left -fill both -ipadx 2m -ipady 1m -expand 1
    pack $f.f2.c1 -side left -fill both -ipadx 2m -ipady 1m
    pack $f.f2.c2 -side left -fill both -ipadx 2m -ipady 1m

    # Make calendar selector box
    make_selection_list $f.cbox $f.clist Calendar
    $f.clist configure -width 20 -height 8
    pack $f.cbox -in $f.f2.c3 -side top -fill both -expand 1

    # Prevent horizontal scrolling in toc
    bind $f.clist <2> {%W scan mark 0 %y}
    bind $f.clist <B2-Motion> {%W scan dragto 0 %y}

    # Make early listing box
    frame $f.early -class Pane
    pack $f.early -in $f.f2.c2 -side top -expand 1 -fill both
    label_widget $f.early {Early Warning}
    scale $f.early.val\
	-from 0 -to 15\
	-length 2i\
	-label Days\
	-tickinterval 5\
	-showvalue 1
    pack $f.early.val -fill y

    # Hiliting
    frame $f.hilite -class Pane
    pack $f.hilite -in $f.f2.c1 -side top -expand 1 -fill both
    label_widget $f.hilite {Highlight}
    set entries {
	{ {Always}		{always}	}
	{ {Never}		{never}		}
	{ {Until Expiration}	{expire}	}
	{ {As Holiday}		{holiday}	}
    }
    set i 1
    foreach e $entries {
	radiobutton $f.hilite.b$i -padx 2m -pady 1m\
	    -text [lindex $e 0]\
	    -variable iedit(hilite)\
	    -value [lindex $e 1]\
	    -anchor w -relief flat

	pack $f.hilite.b$i -side top -fill x

	incr i
    }

    # Todo button
    checkbutton $f.todo -text {Todo Item} -anchor w -padx 2m -pady 1m\
	-variable iedit(todo) -onvalue 1 -offvalue 0
    pack $f.todo -in $f.f2.c1 -side top -fill both

    bind $f <Control-c> {set iedit(done) 0}
    bind $f <Escape> {set iedit(done) 0}
    bind $f <Return>    {set iedit(done) 1}

    wm withdraw $f
}

# Build an editor with arrows
proc iedit_make_editor {w label cmd} {
    frame $w
    label  $w.label -text $label -anchor w -width 14
    label  $w.entry -width 8
    button $w.dleft -bitmap double_left -relief flat
    button $w.sleft -bitmap single_left -relief flat
    button $w.sright -bitmap single_right -relief flat
    button $w.dright -bitmap double_right -relief flat

    $w.dleft   configure -command [concat $cmd [list -10]]
    $w.sleft   configure -command [concat $cmd [list -1]]
    $w.sright  configure -command [concat $cmd [list 1]]
    $w.dright  configure -command [concat $cmd [list 10]]

    pack $w.label  -side left
    pack $w.dleft  -side left
    pack $w.sleft  -side left
    pack $w.entry  -side left
    pack $w.sright -side left
    pack $w.dright -side left
}

# Command for changing start time
proc iedit_change_start {n} {
    global iedit
    set new [expr $iedit(start) + $n]
    set min 0
    set max [expr $iedit(finish) - 30]

    if {$new < $min} {set new $min}
    if {$new > $max} {set new $max}

    set iedit(start) $new
    .iedit.start.entry configure -text [time2text $new]
}

# Command for changing finish time
proc iedit_change_finish {n} {
    global iedit
    set new [expr $iedit(finish) + $n]
    set min [expr $iedit(start) + 30]
    set max [expr 24*60]

    if {$new < $min} {set new $min}
    if {$new > $max} {set new $max}

    set iedit(finish) $new
    .iedit.finish.entry configure -text [time2text $new]
}

proc iedit_fill {item} {
    global iedit
    set f .iedit

    set iedit(item) $item

    # Set-up calendar list
    set cal ""
    catch {set cal [$item calendar]}
    set iedit(calendars) [lsort [ical_filenames]]
    .iedit.clist delete 0 end
    foreach file $iedit(calendars) {
	.iedit.clist insert end [ical_title $file]
	if ![string compare $file $cal] {
	    .iedit.clist selection clear 0 end
	    .iedit.clist selection set end
	}
    }

    # Item text
    $f.text configure -state normal
    $f.text delete 1.0 end
    $f.text insert insert [$item text]
    $f.text configure -state disabled

    # Other stuff
    set iedit(hilite) [$item hilite]
    set iedit(todo)   [$item todo]
    $f.early.val set [$item earlywarning]

    # Appt specific stuff
    if [$item is appt] {
	pack .iedit.fa -before .iedit.bot -side top -fill both -expand 1
	pack .iedit.times -in .iedit.f1\
	    -side right -fill both -expand 1 -ipadx 2m -ipady 1m

	set iedit(start)    [$item starttime]
	set iedit(finish)   [expr [$item starttime] + [$item length]]
	iedit_change_finish 0
	iedit_change_start  0

	if [catch {set alarms [$item alarms]}] {
	    set alarms [cal option DefaultAlarms]
	}
	ruler_settabs .iedit.alarms $alarms
    } else {
	pack forget .iedit.fa
	pack forget .iedit.times
    }
}

proc iedit_save {} {
    global iedit

    set i $iedit(item)
    set early [.iedit.early.val get]

    if {$iedit(todo) != [$i todo]} {$i todo $iedit(todo)}
    if {$early != [$i earlywarning]} {$i earlywarning $early}
    if [string compare $iedit(hilite) [$i hilite]] {$i hilite $iedit(hilite)}

    set sel [.iedit.clist curselection]
    if {[llength $sel] == 1} {
	set old_cal ""
	catch {set old_cal [$i calendar]}
	set cal [lindex $iedit(calendars) [lindex $sel 0]]
	if [string compare $cal $old_cal] {cal add $i $cal}
    }

    if [$i is appt] {
	set s $iedit(start)
	set l [expr $iedit(finish) - $s]
	if {$l < 30} {set l 30}

	if {$s != [$i starttime]} {$i starttime $s}
	if {$l != [$i length]}    {$i length $l}

	set new_alarms [ruler_tabs .iedit.alarms]
	if [catch {set old_alarms [$i alarms]}] {
	    set old_alarms [cal option DefaultAlarms]
	}
	if [string compare $new_alarms $old_alarms] {$i alarms $new_alarms}
    }

    set iedit(item) {}
}
