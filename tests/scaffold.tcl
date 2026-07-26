# Copyright (c) 1996  by Sanjay Ghemawat
proc start_test {category} {
    puts -nonewline stderr [format "%-20s" $category]
}

proc end_test {} {
    puts stderr ""
}

proc run {args} {
    puts -nonewline stderr .
    uplevel $args
}

proc failed {msg} {
    puts stderr "\n=== failure ==="
    puts stderr $msg
}

proc okay {script} {
    if ![catch {uplevel $script} msg] {return}
    failed "error: $msg"
}

proc expect {script value} {
    if [catch {uplevel $script} msg] {
        failed $msg
        return
    }

    if [string compare $msg $value] {
        failed "expected \"$value\", got \"$msg\""
    }
}

proc expect_error {script error} {
    if ![catch {uplevel $script} msg] {
        failed $msg
        return
    }

    if [string compare $msg $error] {
        failed "expected error \"$error\", got \"$msg\""
    }
}

proc init_tests {} {
    global testdir auto_path tmp ical

    set testdir $ical(library)/tests
    set auto_path [concat [list $testdir] $auto_path]
    set tmp icaltests

    catch {exec chmod -R u+w $tmp}
    catch {exec rm -rf $tmp}
    exec mkdir $tmp
}

proc cleanup_tests {} {
    set tmp icaltests
    catch {exec chmod -R u+w $tmp}
    catch {exec rm -rf $tmp}
}
