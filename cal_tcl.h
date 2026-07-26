/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _CAL_TCL_H
#define _CAL_TCL_H

/*
 * TCL Interface to calendar.
 */

#include "object.h"
#include "arrays.h"

class Calendar;
class CalFile;
class Item;
class Item_Tcl;

declareArray(FileList,CalFile*)

class Calendar_Tcl : public Object {
  public:
    Calendar_Tcl(Tcl_Interp*, char const* handle, char const* file);
    virtual ~Calendar_Tcl();

    int error();                /* Used to detect initialization error */
    char const* error_msg();    /* Error description */

    virtual int method(int, const char*[]);

    CalFile*  main;             /* Main calendar */
    FileList* includes;         /* Included calendars */

    void add_item_handles(CalFile*);
    void remove_item_handles(Calendar*);

     /*
      * effects Returns calendar file with specified name.
      *         If name is nil, returns main calendar file.
      *         If no calendar file matches name, returns nil.
      */
    CalFile* name2file(char const* name);

     /*
      * effects Returns calendar file for specified calendar.
      *         If no calendar file matches c, returns nil.
      */
    CalFile* cal2file(Calendar* c);

    /*
     * effects  Purge unnecessary hidden entries from main calendar.
     */
    void purge();

    /*
     * modifies includes/items/error_message.
     * effects  Uses string names in main calendar to fixup include list.
     *          Appends errors to error_message.
     */
    void fix_includes();

    void clear_error();
    void add_error(char const* title, char const* text);
  private:
    int had_error;
    charArray* msg;             /* Always kept null terminated */
};

inline int Calendar_Tcl::error() {
    return had_error;
}

inline char const* Calendar_Tcl::error_msg() {
    return msg->as_pointer();
}

#endif /* _CAL_TCL_H */
