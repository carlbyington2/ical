# Copyright (c) 1996  by Sanjay Ghemawat
if [get_number "" {Select Number} {Number} {Select it} 0 100 20 50 num] {
    error_notify "" "Selected $num" Info
}
