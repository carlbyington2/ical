# Copyright (c) 1996  by Sanjay Ghemawat
if [get_file_name "" {Select File} {Sample file selection} file] {
    error_notify "" "Selected $file" Info
}
