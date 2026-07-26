# Copyright (c) 1996  by Sanjay Ghemawat
# Test class stuff

class Stack {} {
    set slot(elements) ""
}

method Stack size {} {
    return [llength $slot(elements)]
}

method Stack empty {} {
    return [expr {[$self size] == 0}]
}

method Stack push {x} {
    set slot(elements) [linsert $slot(elements) 0 $x]
}

# requires - ![$self empty]
method Stack top {} {
    return [lindex $slot(elements) 0]
}

# requires - ![$self empty]
method Stack pop {} {
    set slot(elements) [lrange $slot(elements) 1 end]
}

subclass MultiStack Stack {} {
    Stack::constructor $self
}

method MultiStack destructor {} {
    Stack::destructor $self
}

method MultiStack push {args} {
    foreach x $args {
	Stack::push $self $x
    }
}

method MultiStack pop_all {} {
    set list {}
    while {![$self empty]} {
	lappend list [$self top]
	$self pop
    }
    return $list
}

# Create a new stack instance and store its handle
# in the TCL variable "stack".
set stack [Stack]

# Push some items on the stack
$stack push 1
$stack push 2
$stack push 3

while {![$stack empty]} {
    puts stdout [$stack top]
    $stack pop
}

# Delete the object
class_kill $stack

# MultiStack test
set stack [MultiStack]

# Push some items on the stack
$stack push 1 2 3 4 5

puts stdout [join [$stack pop_all]]

# Delete the object
class_kill $stack
