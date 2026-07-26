/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _CALENDARH
#define _CALENDARH

#include <stdio.h>
#include "basic.h"

class Item;
class Lexer;
class OptionMap;
class pointerArray;
class UidSet;
class charArray;

class Calendar {
  public:
    Calendar();                 /* Create a new calendar */
    ~Calendar();

    /*
     * Item list.
     */
    int Size() const;                   /* Number of items */
    Item* Get(int) const;               /* Get the ith item */
    void Add(Item*);                    /* Add an item */
    void Remove(Item*);                 /* Remove an item */

    /*
     * Included calendars.
     */
    void Include(char const*);
    void Exclude(char const*);

    int  NumIncludes() const;
    char const* GetInclude(int) const;

    /*
     * True iff this calendar is read-only.
     */
    int ReadOnly() const { return readonly; }
    void SetReadOnly(int t) { readonly = t; }

    /*
     * Read/Write.
     */
    int Read(Lexer*);
    void Write(FILE*) const;

    int Hidden(char const* uid) const;
    // effects - Returns true iff item named by uid should be hidden.

    void Hide(char const* uid);
    // effects - Add specified uid to set of uids of hidden items.

    void RestrictHidden(UidSet const* set);
    // effects - Restrict hidden items to the specified set of items.

    // Options...

    char const* GetOption(char const* key) const;
    // effects  - Return value associated with option named "key".
    //            Returns 0 if option is not found.

    void SetOption(char const* key, char const* value);
    // modifies - this
    // effects  - add "<key, value>" to option list.

    void RemoveOption(char const* key);
    // modifies - this
    // effects  - Remove any option associated with "key"

    bool Visible() const { return GetOption("Visible")[0] == '1'; }

  protected:
    pointerArray& items;                // Items
    pointerArray& includes;             // Included calendars
    int readonly;                       // Readonly calendar?
    UidSet*       hidden;               // Hidden items from other calendars
    OptionMap*    options;              // Calendar options

    void clear();
    // modifies this
    // effects  Restores calendar to pristine state.
};

#endif /* _CALENDARH */
