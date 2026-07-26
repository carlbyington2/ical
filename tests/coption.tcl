# Copyright (c) 1996  by Sanjay Ghemawat
# Calendar option tests

proc copt_tests {} {
    global tmp
    exec rm -f $tmp/file

    start_test "Calendar Options"
    run copt1
    run copt2
    run copt3
    run copt4
    end_test
}

# Check for non-existent option
proc copt1 {} {
    global tmp
    calendar cal $tmp/file
    expect_error {set x [cal option FooBar]} {unknown calendar option}
    cal delete
}

# Check for newly created options
proc copt2 {} {
    global tmp
    calendar cal $tmp/file
    cal option Foo 1
    cal option Bar {Hello\There}
    expect {cal option Foo} 1
    expect {cal option Bar} {Hello\There}
    expect_error {set x [cal option FooBar]} {unknown calendar option}
    cal delete
}

# Check for option modification
proc copt3 {} {
    global tmp
    calendar cal $tmp/file
    cal option FooBar Hello
    expect {cal option FooBar} Hello
    cal option FooBar There
    expect {cal option FooBar} There
    cal delete
}

# Check for option save/restore
proc copt4 {} {
    global tmp

    calendar cal $tmp/file
    cal option FooBar Hello
    cal save
    cal delete

    calendar cal $tmp/file
    expect {cal option FooBar} Hello
    expect_error {set x [cal option OtherOption]} {unknown calendar option}
    cal option FooBar There
    cal save
    cal delete

    calendar cal $tmp/file
    expect {cal option FooBar} There
    expect_error {set x [cal option OtherOption]} {unknown calendar option}
    cal delete
}
