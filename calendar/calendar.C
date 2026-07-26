/* Copyright (c) 1993 by Sanjay Ghemawat */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "basic.h"
#include "calendar.h"
#include "item.h"
#include "lexer.h"
#include "misc.h"
#include "options.h"
#include "uid.h"
#include "version.h"

#include "arrays.h"

// Calendar options with default values
struct OptionDesc {
    char const* key;
    char const* val;
};

static OptionDesc option_list[] = {
    { "DefaultEarlyWarning",    "1"             },
    { "DefaultAlarms",          "0 5 10 15"     },

    { "DayviewTimeStart",       "8"             },
    { "DayviewTimeFinish",      "18"            },
    { "ItemWidth",              "9"             },
    { "NoticeHeight",           "6"             },

    { "AmPm",                   "1"             },
    { "MondayFirst",            "0"             },
    { "AllowOverflow",          "1"             },

    { "Visible",                "1"             },
    { "IgnoreAlarms",           "0"             },
    { "Color",            "<Default> <Default>" },
    { "Timezone",               "<Local>"       },

    { 0,                        0               }
};

static OptionMap* option_default = 0;

Calendar::Calendar()
     : items(*(new pointerArray)),
       includes(*(new pointerArray))
{
    // Initialize default option map if not done already
    if (option_default == 0) {
        option_default = new OptionMap;
        for (int i = 0; option_list[i].key != 0; i++) {
            option_default->store(option_list[i].key, option_list[i].val);
        }
    }
    
    readonly = 0;
    hidden = new UidSet;
    options = new OptionMap;
}

Calendar::~Calendar() {
    clear();
    delete &items;
    delete &includes;
    delete hidden;
    delete options;
}

void Calendar::clear() {
    int i;

    for (i = 0; i < items.size(); i++) {
        Item* item = (Item*) items[i];
        delete item;
    }
    items.clear();

    for (i = 0; i < includes.size(); i++) {
        char* includeName = (char*) includes[i];
        delete includeName;
    }
    includes.clear();

    for (UidSet_Elements h = hidden; h.ok(); h.next()) {
        delete [] (char*)(h.get());
    }
    hidden->clear();

    delete options;
    options = new OptionMap;

    readonly = 0;
}

void Calendar::Add(Item* item) {
    items.append((void*) item);
}

void Calendar::Remove(Item* item) {
    for (int i = 0; i < items.size(); i++) {
        if (items[i] == (void*) item) {
            /* Found it */

            /* Shift the other items over */
            for (int j = i + 1; j < items.size(); j++)
                items[j - 1] = items[j];
            items.remove();
            break;
        }
    }
}

int Calendar::Read(Lexer* lex) {
    clear();

    if (lex->Status() == Lexer::Error) {
        /* No input file at all */
        return 1;
    }

    int file_major, file_minor;
    const char* modifier;

    if (! lex->SkipWS() ||
        ! lex->Skip("Calendar") ||
        ! lex->SkipWS()) {
        lex->SetError("file does not contain calendar");
        return 0;
    }

    /*
     * Get file version number.
     * We can only understand version numbers that have a major
     * component <= to my major version number and >= 2.
     * Version 1 is not supported anymore.
     */

    if (! lex->Skip('[') ||
        ! lex->Skip('v') ||

        ! lex->GetNumber(file_major) ||
        ! (file_major >= 2) ||
        ! (file_major <= VersionMajor) ||

        ! lex->Skip('.') ||

        ! lex->GetNumber(file_minor) ||
        ! (file_minor >= 0) ||

        ! lex->GetUntil(']', modifier) ||
        /* Possibly check modifier here */

        ! lex->Skip(']')) {
        lex->SetError("illegal version");
    }

    while (1) {
        char c;

        lex->SkipWS();
        lex->Peek(c);

        switch (lex->Status()) {
          case Lexer::Eof:
            return 1;
          case Lexer::Error:
            return 0;
          default:
            break;
        }

        char const* keyword;

        if (! lex->GetId(keyword) ||
            ! lex->SkipWS() ||
            ! lex->Skip('[')) {
            lex->SetError("error reading item header");
            return 0;
        }

        if (strcmp(keyword, "Appt") == 0) {
            Item* item = new Appointment;
            if (! item->Read(lex)) {
                delete item;
                return 0;
            }
            Add(item);
        }
        else if (strcmp(keyword, "Note") == 0) {
            Item* item = new Notice;
            if (! item->Read(lex)) {
                delete item;
                return 0;
            }
            Add(item);
        }
        else if (strcmp(keyword, "IncludeCalendar") == 0) {
            /* Read the name */
            char const* name;
            if (!lex->GetString(name)) {
                lex->SetError("error reading included file name");
                return 0;
            }
            Include(name);
        }
        else if (strcmp(keyword, "Hide") == 0) {
            char const* x;
            if (!lex->SkipWS() || !lex->GetUntil(']', x)) {
                lex->SetError("error reading hidden item uid");
                return 0;
            }
            Hide(x);
        }
        else {
            // Stash away the keyword because lex->GetString overwrites it.
            char* key = copy_string(keyword);
            char const* val;

            if (!lex->GetString(val)) {
                lex->SetError("error reading calendar property");
                delete [] key;
                return 0;
            }

            // Enter option into this calendar
            options->store(key, val);

            delete [] key;
        }

        if (! lex->SkipWS() ||
            ! lex->Skip(']')) {
            lex->SetError("incomplete item");
            return 0;
        }
    }
}

void Calendar::Write(FILE* file) const {
    charArray* out = new charArray;

    format(out, "Calendar [v%d.%d]\n", VersionMajor, VersionMinor);
    options->write(out);
    for (int i = 0; i < includes.size(); i++) {
        char const* name = (char const*) includes[i];
        append_string(out, "IncludeCalendar [");
        Lexer::PutString(out, name);
        append_string(out, "]\n");
    }
    for (int i = 0; i < items.size(); i++) {
        Item* item = (Item*) items[i];

        if (item->AsNotice() != 0) {
            append_string(out, "Note [\n");
        }
        else {
            append_string(out, "Appt [\n");
        }
        item->Write(out);
        append_string(out, "]\n");
    }

    for (UidSet_Elements h = hidden; h.ok(); h.next()) {
        format(out, "Hide [%s]\n", h.get());
    }

    // Just dump array out to file.
    out->append('\0');
    fputs(out->as_pointer(), file);
    delete out;
}

int Calendar::Size() const {
    return items.size();
}

void Calendar::Include(char const* name) {
    includes.append((void*) copy_string(name));
}

void Calendar::Exclude(char const* name) {
    for (int i = 0; i < includes.size(); i++) {
        if (strcmp(name, ((char const*) includes[i])) == 0) {
            char* includeName = (char*) includes[i];
            delete includeName;

            /* Shift other includes over */
            for (int j = i + 1; j < includes.size(); j++) {
                includes[j - 1] = includes[j];
            }
            includes.remove();
            return;
        }
    }
}

int Calendar::NumIncludes() const {
    return includes.size();
}

char const* Calendar::GetInclude(int i) const {
    return ((char const*) includes[i]);
}

Item* Calendar::Get(int i) const {
    return (Item*) items[i];
}

int Calendar::Hidden(char const* uid) const {
    return (hidden->contains(uid));
}

void Calendar::Hide(char const* uid) {
    hidden->insert(copy_string(uid));
}

void Calendar::RestrictHidden(UidSet const* set) {
    UidSet* old = new UidSet;

    // Collect list of hide entries that can be removed 
    for (UidSet_Elements h = hidden; h.ok(); h.next()) {
        if (!set->contains(h.get())) {
            old->insert(h.get());
        }
    }

    // Remove the collected entries.
    for (UidSet_Elements x = old; x.ok(); x.next()) {
        hidden->remove(x.get());
        delete [] (char*)(x.get());
    }

    delete old;
}

char const* Calendar::GetOption(char const* key) const {
    char const* val;
    if (options->fetch(key, val)) return val;
    if (option_default->fetch(key, val)) return val;
    return 0;
}

void Calendar::SetOption(char const* key, char const* val) {
    options->store(key, val);
}

void Calendar::RemoveOption(char const* key) {
    options->remove(key);
}
