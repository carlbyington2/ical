/* Copyright (c) 1993 by Sanjay Ghemawat */

#include <sys/types.h>
#include <sys/file.h>
#include <stdlib.h>

#include <stddef.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "basic.h"
#include "config.h"

#include "calfile.h"
#include "calendar.h"
#include "lexer.h"
#include "misc.h"
#include "statfix.h"
#include "uid.h"

#include <unistd.h>

// Only use "fsync" if it is available.
#ifdef HAVE_FSYNC
extern "C" int fsync(int);
#else
static int fsync(int) {return 0;}
#endif

// Get various file names
static char const* home_backup_file();  // Backup file in home dir
static char const* tmp_backup_file();   // Backup file in tmp dir

static int backup_file(char const* src, char const* dst, long mode);
// effects      Backup file named "src" to file named "dst".
//              Change the mode of "dst" to "mode".
//              Return true iff successful.

const char* CalFile::lastError = "no error";

CalFile::CalFile(int ro, const char* name) {
    readOnly = ro;

    int len = strlen(name);
    char* tmp;

    // Copy file name
    tmp = new char[len+1];
    strcpy(tmp, name);
    fileName = tmp;

    // Copy backup file name
    tmp = new char[len+2];
    sprintf(tmp, "%s~", fileName);
    backupName = tmp;

    // Get directory name for access checks
    char* lastSlash = strrchr(const_cast<char*>(name), '/');
    if (lastSlash == 0) {
        /* Calendar is in current directory */
        tmp = new char[3];
        strcpy(tmp, "./");
    }
    else {
        int dirlen = lastSlash + 1 - name;
        tmp = new char[dirlen+1];
        strncpy(tmp, name, dirlen);
        tmp[dirlen] = '\0';
    }
    dirName = tmp;

    // Get temporary file name.  Make sure it works even
    // on systems with a 14 character file name limit.
    tmp = new char[strlen(dirName)+20];
    sprintf(tmp, "%sical%d~", dirName, getpid());
    tmpName = tmp;

    lastModifyValid = 0;

    calendar = new Calendar;
    calendar->SetReadOnly(readOnly);

    modified = 0;
}

CalFile::~CalFile() {
    delete calendar;
    delete fileName;
    delete backupName;
    delete tmpName;
    delete dirName;
}

void CalFile::Modified() {
    modified = 1;
}

int CalFile::Write() {
    // Get information about the calendar file
    struct stat buf;
    int is_slink = 0;

    int result = lstat(fileName, &buf);
    if ((result >= 0) && S_ISLNK(buf.st_mode)) {
        // Get mode for real referenced file
        is_slink = 1;
        result = stat(fileName, &buf);
    }

    if (result < 0) {
        /* Could not get file mode */
        if (errno == ENOENT) {
            /* Original file does not even exist - try to write directly */
            if (WriteTo(calendar, fileName)) {
                written();
                return 1;
            }
        }
        return 0;
    }

    long mode = buf.st_mode & 07777;

    // See if file is a link, or if the containing directory is write-protected
    if (is_slink || (buf.st_nlink > 1) || (access(dirName, W_OK) < 0)) {
        // Backup by copying to preserve links
        return WriteInPlace(mode);
    }
    else {
        // Backup by renaming old version if possible
        return WriteNew(mode);
    }
}

// Write calendar to temporary location, rename the current version
// to a backup file and then rename the new version to the right
// file name.

int CalFile::WriteNew(long mode) {
    if (! WriteTo(calendar, tmpName)) {
        unlink(tmpName);
        return 0;
    }

    if (chmod(tmpName, mode) < 0) {
        /* Could not set new file mode */
        lastError = strerror (errno);
        unlink(tmpName);
        return 0;
    }

    // We could conceivably do more sanity checks here.

    // Create backup file.
    // We could check for errors and fail here, but that seems too paranoid.
    unlink(backupName);
    link(fileName, backupName);

    // Now rename the new version
    if (rename(tmpName, fileName) < 0) {
        lastError = strerror (errno);
        unlink(tmpName);
        return 0;
    }

    written();
    return 1;
}

// First backup the current stable version of the calendar, and then
// write a new version in place.

int CalFile::WriteInPlace(long mode) {
    // XXX Should we just ignore errors while making a backup?
    CopyBackup(mode);

    if (WriteTo(calendar, fileName)) {
        written();
        return 1;
    }

    return 0;
}

int CalFile::CopyBackup(long mode) {
    // Try backing up in various place until we succeed:
    //
    // * backupName
    // * In home directory
    // * In tmp directory

    if (backup_file(fileName, backupName,          mode)) return 1;
    if (backup_file(fileName, home_backup_file(),  mode)) return 1;
    if (backup_file(fileName, tmp_backup_file(),   mode)) return 1;

    return 0;
}

int CalFile::Read() {
    Calendar* old = ReRead();
    if (old != 0) {
        delete old;
        return 1;
    }
    return 0;
}

Calendar* CalFile::ReRead() {
    Time newFileTime;
    int gotTime = GetModifyTime(fileName, newFileTime);

    Calendar* cal = ReadFrom(fileName);
    Calendar* old = 0;
    if (cal != 0) {
        old = calendar;
        calendar = cal;
        calendar->SetReadOnly(readOnly);

        PerformAccessCheck();

        modified = 0;

        lastModifyValid = gotTime;
        if (gotTime)
            lastModifyTime = newFileTime;
    }
    return old;
}

int CalFile::FileHasChanged() {
    PerformAccessCheck();

    Time newModifyTime;
    int newModifyValid = GetModifyTime(fileName, newModifyTime);

    if (newModifyValid != lastModifyValid) {
        /* Ability to read file information changed */
        return 1;
    }

    return (lastModifyValid && (newModifyTime != lastModifyTime));
}

Calendar* CalFile::ReadFrom(const char* name) {
    Calendar* cal = new Calendar;
    Lexer input(name);

    if (! cal->Read(&input)) {
        lastError = input.LastError();
        delete cal;
        cal = 0;
    }

    return cal;
}

int CalFile::WriteTo(Calendar* cal, const char* name) {
    FILE* output = fopen(name, "w");
    if (! output) {
        lastError = "could not open file for writing calendar";
        return 0;
    }

    cal->Write(output);
    fflush(output);
    if (ferror(output) || (fsync(fileno(output)) < 0)) {
        lastError = "error writing calendar file";
        fclose(output);
        return 0;
    }

    fclose(output);
    return 1;
}

int CalFile::GetModifyTime(char const* file, Time& t) {
    struct stat buf;

    int ret = stat(file, &buf);
    if (ret < 0) return 0;

    struct timeval tv;
    tv.tv_sec  = buf.st_mtime;
    tv.tv_usec = 0;

    t = Time(tv);
    return 1;
}

void CalFile::PerformAccessCheck() {
    calendar->SetReadOnly(readOnly);

    if (access(fileName, W_OK) < 0) {
        switch (errno) {
          case ENOENT:
            /* File does not exist */
            break;
          case EACCES:
          case EROFS:
          case ETXTBSY:
            /* Permission denied */
            calendar->SetReadOnly(1);
            break;
          default:
            /* Should not happen if we were successfuly able to read cal */
            break;
        }
    }
}

void CalFile::written() {
    modified = 0;
    lastModifyValid = GetModifyTime(fileName, lastModifyTime);
}

static int backup_file(char const* src, char const* dst, long mode) {
    if (dst == 0) return 0;
    if (!copy_file(src, dst)) return 0;

    // XXX Ignoring error while changing mode of backup file
    chmod(dst, mode);
    return 1;
}

static char const* home_backup_file() {
    static char const* part_name = "icalbak~";
    static char const* full_name = 0;
    static int inited = 0;

    // Make sure we initialize only once
    if (!inited) {
        inited = 1;
        char const* home = getenv("HOME");
        if (home != 0) {
            char* copy = new char[strlen(home) + strlen(part_name) + 2];
            sprintf(copy, "%s/%s", home, part_name);
            full_name = copy;
        }
    }

    return full_name;
}

static char const* tmp_backup_file() {
    static char const* prefix = "/tmp/ical_";
    static char const* full_name = 0;
    static int inited = 0;

    // Make sure we initialize only once
    if (!inited) {
        inited = 1;
        char const* uid = my_name();
        if (uid != 0) {
            char* copy = new char[strlen(prefix) + strlen(uid) + 2];
            sprintf(copy, "%s%s~", prefix, uid);
            full_name = copy;
        }
    }

    return full_name;
}
