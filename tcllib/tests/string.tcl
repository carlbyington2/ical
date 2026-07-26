# Copyright (c) 1996  by Sanjay Ghemawat
if [get_string "" {Enter String} {Sample string dialog} XXXX result] {
    error_notify "" "Entered $result" Info
}
