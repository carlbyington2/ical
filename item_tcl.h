/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _ITEM_TCL_H
#define _ITEM_TCL_H

/*
 * TCL Interface to items.
 */

#include "object.h"
#include "item.h"

class CalFile;

class Item_Tcl : public Object {
  public:
    Item_Tcl(Tcl_Interp*, Item*, CalFile*);
    virtual ~Item_Tcl();

    // effects  Return associated item */
    Item* value();

    // effects  Return associated calendar file
    CalFile* calendar();

    // effects  Set associated calendar fi;
    void set_calendar(CalFile*);

    // effects  Returns Item_Tcl for specified item.  If no such
    //          Item_Tcl exists, returns 0.
    //
    static Item_Tcl* find(Item*);

    // effects  Handle TCL commands
    virtual int method(int, const char*[]);
  private:
    Item* item;         // Associated item
    CalFile* cal;       // Associated calendar
};

inline Item* Item_Tcl::value() {
    return item;
}

inline CalFile* Item_Tcl::calendar() {
    return cal;
}

inline void Item_Tcl::set_calendar(CalFile* c) {
    cal = c;
}

#endif /* _ITEM_TCL_H */
