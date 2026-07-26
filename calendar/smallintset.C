/* Copyright (c) 1993 by Sanjay Ghemawat */

#include <ctype.h>
#include "smallintset.h"
#include "arrays.h"
#include "lexer.h"
#include "misc.h"

unsigned int SmallIntSet::Size() const {
    unsigned int size = 0;

    for (int i = 0; i <= SISetLargestMember; i++)
        if (Member(i)) size++;
    return size;
}
int SmallIntSet::Read(Lexer* lex) {
    Clear();
    while (1) {
        char c;

        lex->SkipWS();
        if (! lex->Peek(c)) return 0;
        if (!isdigit(c)) return 1;

        int num;
        if (! lex->GetNumber(num)) return 0;
        if ((num < 0) || (num > SISetLargestMember)) {
            lex->SetError("illegal smallintset member");
            return 0;
        }
        Insert(num);
    }
}

void SmallIntSet::Write(charArray* out) const {
    for (int i = 0; i <= SISetLargestMember; i++) {
        if (Member(i)) {
            format(out, " %d", i);
        }
    }
}
