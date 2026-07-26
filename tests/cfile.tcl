# Copyright (c) 1996  by Sanjay Ghemawat
# Calendar file writing tests

# XXX Need tests for "cal modified" and "cal stale".

proc cfile_tests {} {
    start_test "Calendar Files"
    run cfile1
    run cfile2
    run cfile3
    run cfile4
    run cfile5
    run cfile6
    run cfile7
    run cfile8
    run cfile9
    end_test
}

# Save an empty calendar file and make sure it can be read back
proc cfile1 {} {
    global tmp

    make_calendar $tmp/file1
    check_calendar $tmp/file1 TestValue
}

# Make sure backup is being made correctly
proc cfile2 {} {
    global tmp

    make_calendar $tmp/file1
    exec chmod 0700 $tmp/file1

    file stat $tmp/file1 stat
    set inode $stat(ino)
    set mode $stat(mode)

    make_backup $tmp/file1
    check_calendar $tmp/file1 NewValue
    check_calendar $tmp/file1~ TestValue

    # Check that backup is just the original renamed file
    file stat $tmp/file1~ stat
    expect {set stat(ino)} $inode
    expect {set stat(mode)} $mode

    # Check that new file has same mode as original
    file stat $tmp/file1 stat
    expect {set stat(mode)} $mode
}

# Using calendar in read-protected directory
proc cfile3 {} {
    global tmp

    catch {exec chmod 0700 $tmp/rp}
    exec rm -rf $tmp/rp
    exec mkdir $tmp/rp
    exec chmod 0200 $tmp/rp
    expect_error\
        {make_backup $tmp/rp/file}\
        {permission denied}

    exec chmod 0700 $tmp/rp
}

# Using calendar in write-protected directory
proc cfile4 {} {
    global tmp env

    exec rm -rf $tmp/rp
    exec mkdir $tmp/rp
    make_calendar $tmp/rp/file
    exec chmod 0500 $tmp/rp
    okay {make_backup $tmp/rp/file}
    check_calendar $env(HOME)/icalbak~ TestValue
    exec rm -f $env(HOME)/icalbak~
    exec chmod 0700 $tmp/rp
}

# Using symbolically linked calendar
proc cfile5 {} {
    global tmp

    eval exec rm -f [glob $tmp/file*]
    make_calendar $tmp/file1
    exec ln -s file1 $tmp/file2
    check_calendar $tmp/file2 TestValue
    make_backup $tmp/file2
    check_calendar $tmp/file1 NewValue
    check_calendar $tmp/file2 NewValue
    check_calendar $tmp/file2~ TestValue
}

# Using hard linked calendar
proc cfile6 {} {
    global tmp

    eval exec rm -f [glob $tmp/file*]
    make_calendar $tmp/file1
    exec ln $tmp/file1 $tmp/file2
    check_calendar $tmp/file2 TestValue
    make_backup $tmp/file2
    check_calendar $tmp/file1 NewValue
    check_calendar $tmp/file2 NewValue
    check_calendar $tmp/file2~ TestValue
}

# Backup when dir is write protected and HOME is not available
# Using calendar in write-protected directory
proc cfile7 {} {
    global tmp env

    set me [exec whoami]

    exec rm -rf $tmp/rp
    exec mkdir $tmp/rp
    make_calendar $tmp/rp/file
    exec chmod 0500 $tmp/rp

    exec touch $env(HOME)/icalbak~
    exec chmod 0 $env(HOME)/icalbak~
    okay {make_backup $tmp/rp/file}
    exec chmod 0600 $env(HOME)/icalbak~

    expect_error\
        {check_calendar $env(HOME)/icalbak~ TestValue}\
        "$env(HOME)/icalbak~: file does not contain calendar\n"

    check_calendar /tmp/ical_$me~ TestValue
    exec rm -f /tmp/ical_$me~ $env(HOME)/icalbak~
    exec chmod 0700 $tmp/rp
}

# When all backups fail
proc cfile8 {} {
    global tmp env

    set me [exec whoami]

    exec rm -rf $tmp/rp
    exec mkdir $tmp/rp
    make_calendar $tmp/rp/file
    exec chmod 0500 $tmp/rp

    exec touch $env(HOME)/icalbak~
    exec chmod 0 $env(HOME)/icalbak~
    exec touch /tmp/ical_$me~
    exec chmod 0 /tmp/ical_$me~
    okay {make_backup $tmp/rp/file}
    exec chmod 0600 $env(HOME)/icalbak~
    exec chmod 0600 /tmp/ical_$me~

    expect_error\
        {check_calendar $env(HOME)/icalbak~ TestValue}\
        "$env(HOME)/icalbak~: file does not contain calendar\n"

    expect_error\
        {check_calendar /tmp/ical_$me~ TestValue}\
        "/tmp/ical_$me~: file does not contain calendar\n"

    exec rm -f /tmp/ical_$me~ $env(HOME)/icalbak~
    exec chmod 0700 $tmp/rp
}

# Saving items with % signs
proc cfile9 {} {
    global tmp
    set fname $tmp/file1
    set today [date today]

    # Create an item with a text field that has a % sign
    exec rm -f $fname
    calendar cal $fname
    set item [notice]
    $item text "a % b"
    $item date $today
    cal add $item
    cal save
    cal delete

    # Check the item
    calendar cal $fname
    cal query -all $today $today item junk {
        if [string compare [$item text] "a % b"] {
            failed "item text containing % sign was mangled"
        }
    }
    cal delete

    exec rm -f $fname
}

proc make_calendar {fname} {
    exec rm -f $fname
    calendar cal $fname
    cal option TestProperty TestValue
    cal save
    cal delete
}

proc make_backup {fname} {
    calendar cal $fname
    cal option TestProperty NewValue
    cal save
    cal delete
}

proc check_calendar {fname propvalue} {
    calendar cal $fname
    expect {cal option TestProperty} $propvalue
    cal delete
}
