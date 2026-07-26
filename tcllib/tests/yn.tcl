# Copyright (c) 1996  by Sanjay Ghemawat
if [yes_or_no "" "Yes or No?" Yeah!!! Nope.] {
    error_notify "" "Yes!!!" Info
} else {
    error_notify "" "Nah..." Info
}
