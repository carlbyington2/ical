/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _COLLECT_H
#define _COLLECT_H

/*
 * Collect item occurrences.
 */

#include "Array.h"
#include "Date.h"

class Item_Tcl;
class Calendar_Tcl;
class Calendar;

/*
 * Item listing sorted by occurrence date.
 */

struct Occurrence {
    Item_Tcl* item;
    Date      date;

    // The following are needed because older versions of g++ (before 2.3.3)
    // do not seem to generate default constructors correctly

    Occurrence() { }
    Occurrence(Occurrence const& o) {
        item = o.item;
        date = o.date;
    }
    Occurrence& operator = (Occurrence const& o) {
        item = o.item;
        date = o.date;
        return *this;
    }
};

declareArray(ItemList,Item_Tcl*)
declareArray(Occurrences,Occurrence)

extern void collect_all(Calendar_Tcl*, ItemList& list, bool only_visible);
// modifies     list
// effects      Collect together all of the items contained in
//              all (or only in visible) calendar files and append them to "list".

extern void collect_calendar(Calendar_Tcl*, Calendar* cal, ItemList& list);
// modifies     list
// effects      Append all of the items contained in "cal" to "list".

/*
 * Fill list with any occurrences in specified range of the items
 * mentioned in "items".
 *
 * If earlywarning is true, also consider items whose early warning
 * period falls in the specified range.
 */
extern void collect_occurrences(Calendar_Tcl* tcl,
                                ItemList const& items,
                                Occurrences& list,
                                Date start,
                                Date finish,
                                int  earlywarning);

/*
 * Sort occurrences.
 */
extern void sort_occurrences(Occurrences&);

/*
 * Reverse occurrences.
 */
extern void reverse(Occurrences&);

#endif /* _COLLECT_H */
