/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _OBJECT_H
#define _OBJECT_H

/*
 * C++ Objects that can be manipulated by TCL code.
 *
 * 1. Each object has a string handle and a string type.
 * 2. The string handle is used in TCL code to name the
 *    object.
 * 3. The handle can be invoked as a TCL command.  The command
 *    dispatches to the "method" operation in the TCL command.
 */

extern "C" {
#include <tcl.h>
}

class Object {
  public:
    Object(Tcl_Interp*, char const* type);
    Object(Tcl_Interp*, char const* type, char const* handle);
    virtual ~Object();

    /*
     * effects  Returns associated interpreter.
     */
    Tcl_Interp* tcl();

    /*
     * effects  Return type.
     */
    char const* type();

    /*
     * effects  Returns handle name.  Returned storage is guaranteed
     *          to last as long as the object lives.
     */
    char const* handle();

    /*
     * effects  Returns object with specified handle.  If no such
     *          object exists, returns 0.
     */
    static Object* find(Tcl_Interp*, char const* handle);

    /* TCL Callback */
    virtual int method(int argc, const char* argv[]);
  private:
    void init(Tcl_Interp*, char const* otype, char const* ohandle);

    char* objtype;      /* My type */
    char* name;         /* Handle */
    Tcl_Interp* interp; /* Interpreter */
};

inline Tcl_Interp* Object::tcl() {
    return interp;
}

inline char const* Object::type() {
    return objtype;
}

inline char const* Object::handle() {
    return name;
}

#endif /* _OBJECT_H */
