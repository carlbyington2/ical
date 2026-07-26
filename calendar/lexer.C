/* Copyright (c) 1996  by Sanjay Ghemawat */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "basic.h"
#include "arrays.h"
#include "lexer.h"

char const* Lexer::lastError = "";

Lexer::Lexer(char const* file) {
    tmp = new charArray;

    /* Dummy buffer for errors */
    buf = new char[1];
    buf[0] = '\0';
    index = 0;
    length = 0;

    int fd = open((char*)file, O_RDONLY, 0);
    if (fd < 0) {
        SetError("could not open file");
        return;
    }

    struct stat fs;
    if (fstat(fd, &fs) < 0) {
        SetError("could not get file size");
        return;
    }

    /* Read the contents of the file */
    delete [] buf;
    buf = new char[fs.st_size+1];
    length = 0;
    while (length < fs.st_size) {
        int result = read(fd, buf+length, fs.st_size - length);
        if (result == 0) {
            /* Early EOF! */
            SetError("could not read file");
            break;
        }
        if (result < 0) {
            /* Ignore EINTR errors */
            if (errno == EINTR) {
                continue;
            }
            SetError("could not read file");
            break;
        }
        length += result;
    }

    /* Null-terminate the array for fast scanning */
    buf[fs.st_size] = '\0';

    /* Close the file */
    close(fd);
}

Lexer::Lexer(charArray* text) {
    tmp = new charArray;
    index = 0;
    length = text->size();
    buf = new char[length+1];
    strncpy(buf, text->as_pointer(), length);
    buf[length] = '\0';
}

Lexer::~Lexer() {
    delete [] buf;
    delete tmp;
}

int Lexer::Skip(char const* str) {
    int len = strlen(str);

    if ((index + len) <= length) {
        /* Still have enough chars left */
        if (strncmp(buf+index, str, len) == 0) {
            index += len;
            return 1;
        }
        SetError("unexpected string");
        return 0;
    }

    /* EOF */
    index = length;
    return 0;
}

int Lexer::SkipWS() {
    while (index < length) {
        char c = buf[index];
        if (!isspace(c)) return 1;
 index++;
    }

    return 0;
}

static inline int is_letter(char c) {
    return ((c == '_') || isalpha(c));
}

static inline int is_numletter(char c) {
    return ((c == '_') || isalnum(c));
}

int Lexer::GetId(char const*& x) {
    if (index >= length) {
        return 0;
    }

    /* Check first character */
    if (! is_letter(buf[index])) {
        SetError("illegal character when expecting id");
        return 0;
    }

    int start = index;
    while ((index < length) && is_numletter(buf[index])) {
        index++;
    }

    tmp->clear();
    tmp->concat(buf+start, index - start);
    tmp->append('\0');
    x = tmp->as_pointer();
    return 1;
}

int Lexer::GetUntil(char terminator, char const*& x) {
    if (index >= length) {
        return 0;
    }

    int start = index;
    while ((index < length) && (buf[index] != terminator)) {
        index++;
    }

    tmp->clear();
    tmp->concat(buf+start, index - start);
    tmp->append('\0');
    x = tmp->as_pointer();
    return 1;
}
 
int Lexer::GetNumber(int& x) {
    if (index >= length) {
        return 0;
    }

    char* finish;
    long result = strtol(buf+index, &finish, 10);
    if (finish == (buf+index)) {
        SetError("error while reading number");
        return 0;
    }

    x = result;
    index = (finish - buf);
    return 1;
}

int Lexer::GetString(char const*& x) {
    if (index >= length) return 0;

    tmp->clear();

    while ((index < length) && (buf[index] != ']')) {
        if (buf[index] == '\\') {
            index++;
            if (index >= length) return 0;
        }
        tmp->append(buf[index]);
        index++;
    }

    tmp->append('\0');
    x = tmp->as_pointer();
    return 1;
}

void Lexer::PutString(charArray* out, char const* x) {
    while (*x != '\0') {
        char c = *x;
        if ((c == '\\') || (c == '[') || (c == ']')) out->append('\\');
        out->append(c);
        x++;
    }
}
