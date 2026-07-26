/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _HANDLE_H
#define _HANDLE_H

/*
 * Tcl handles for C++ objects.  A handle is a name (either automatically
 * generated, or provided by the user).  A C++ object is represented
 * as a void*.  Strings representing names are allocated internally by
 * HandleTable, and can be used outside until the corresponding binding
 * is removed, or the table is deleted.
 */

#include <tcl.h>

class HandleTable {
  public:
    HandleTable();
    ~HandleTable();

    /*
     * requires	*this does not contain mapping for handle or object.
     * modifies	*this
     * effects	Adds binding (handle <-> object) to *this.
     */
    void bind(char* handle, void* object);

    /*
     * effects	If *this contains binding (handle <-> x), returns x,
     *		else returns 0.
     */
    void* object(char* handle);

    /*
     * effects	If *this contains binding (x <-> object), returns x,
     *		else returns 0.
     */
    char* handle(void* object);

    /*
     * modifies	*this
     * effects	If *this contains (handle <-> object), removes it.
     */
    void remove(char* handle, void* object);
  private:
    Tcl_HashTable handleMap;	/* Map from handle to object */
    Tcl_HashTable objectMap;	/* Map from object to handle */
};

#endif /* _HANDLE_H */
