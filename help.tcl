# Copyright (c) 1994 by Sanjay Ghemawat
##############################################################################
# Ical on-line help/about

set about(done) 0
proc show_about {leader} {
    global ical about

    set t .about
    if ![winfo exists $t] {
        toplevel $t
        set font1 [pref largeHeadingFont]
        set font2 [pref smallHeadingFont]

        frame $t.top -class Pane
        frame $t.top.author
        label $t.top.version -font $font1 -text "Ical Version $ical(version)"

        label $t.top.author.l1 -font $font2 -text "Written by Sanjay Ghemawat"
        label $t.top.author.l2  -font $font2 -text $ical(author) -anchor e
        pack $t.top.author.l1 -side top -expand 1 -fill x
        pack $t.top.author.l2 -side top -expand 1 -fill x

        pack $t.top.version -side top -expand 1 -fill x -padx 5m -pady 5m
        pack $t.top.author  -side top -expand 1 -fill x -padx 5m -pady 5m

        make_buttons $t.bot 0 {
            {{Okay}             {set about(done) 1}}
        }

        pack $t.top -side top -expand 1 -fill x
        pack $t.bot -side bottom -expand 1 -fill x

        wm title $t {About Ical}
        wm protocol $t WM_DELETE_WINDOW {set about(done) 1}
        bind $t <Control-c> {set about(done) 1}
        bind $t <Return>    {set about(done) 1}

        wm withdraw $t
        update idletasks
    }

    set about(done) 0
    dialog_run $leader $t about(done)
    return
}
