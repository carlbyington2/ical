# Copyright (c) 1996  by Sanjay Ghemawat
# Dialog tests

proc dialog_tests {} {
    start_test "Dialogs"

    run dg_bug

    # Test dialogs with and without a leader window
    global leader

    set leader ""
    run dg_error
    run dg_file
    run dg_num
    run dg_string
    run dg_yn
    run dg_ync

    set leader .foobar
    toplevel .foobar
    pack [canvas .foobar.c -width 8i -height 8i]
    update

    run dg_error
    run dg_file
    run dg_num
    run dg_string
    run dg_yn
    run dg_ync

    destroy .foobar

    end_test
}

proc dg_bug {} {
    after 500 {set bug(done) cancel}
    expect {bug_notify cat foo "Test message"} {}
}

proc dg_error {} {
    global leader
    after 500 {set error_done 1}
    expect {error_notify $leader {This is a test} "Error Test"} {}
}

proc dg_file {} {
    global leader
    after 500 {set file_done 0}
    expect {get_file_name $leader "Test" "Testing file selector" fname /tmp/} 0

    after 500 {set file_done 1}
    expect {get_file_name $leader "Test" "Testing file selector" fname /tmp/} 1
    expect {set fname} /tmp/
}

proc dg_num {} {
    global leader
    after 500 {set num_done 0}
    expect {get_number $leader "Test" "Testing number selector" X 0 100 10 50 result} 0

    after 500 {set num_done 1}
    expect {get_number $leader "Test" "Testing number selector" X 0 100 10 50 result} 1
    expect {set result} 50
}

proc dg_string {} {
    global leader
    after 500 {set str_done 0}
    expect {get_string $leader "Test" "Testing string entry" Foobar result} 0

    after 500 {set str_done 1}
    expect {get_string $leader "Test" "Testing string entry" Foobar result} 1
    expect {set result} Foobar
}

proc dg_yn {} {
    global leader
    after 500 {set yn_done 0}
    expect {yes_or_no $leader "Testing yes/no dialog"} 0

    after 500 {set yn_done 1}
    expect {yes_or_no $leader "Testing yes/no dialog"} 1
}

proc dg_ync {} {
    global leader
    after 500 {set ync_done cancel}
    expect {yes_no_cancel $leader "Testing yes/no/cancel dialog"} cancel

    after 500 {set ync_done no}
    expect {yes_no_cancel $leader "Testing yes/no/cancel dialog"} no

    after 500 {set ync_done yes}
    expect {yes_no_cancel $leader "Testing yes/no/cancel dialog"} yes
}
