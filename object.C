/* Copyright (c) 1993 by Sanjay Ghemawat */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic.h"
#include "ical.h"
#include "object.h"

static int obj_handle(ClientData, Tcl_Interp*, int, const char*[]);

Object::Object(Tcl_Interp* t, char const* type) {
    /* Generate handle */
    static int index = 0;
    char buffer[100];
    index++;
    sprintf(buffer, "h%d", index);
    init(t, type, buffer);
}

Object::Object(Tcl_Interp* t, char const* type, char const* h) {
    init(t, type, h);
}

void Object::init(Tcl_Interp* t, char const* ty, char const* h) {
    objtype = new char[strlen(ty)+1];
    strcpy(objtype, ty);

    name = new char[strlen(h)+1];
    strcpy(name, h);

    interp = t;

    /* Create TCL Command */
    Tcl_CreateCommand(interp, name, obj_handle, (ClientData) this, NULL);
}

Object::~Object() {
    Tcl_DeleteCommand(interp, name);

    delete [] objtype;
    delete [] name;
}

Object* Object::find(Tcl_Interp* tcl, char const* h) {
    Tcl_CmdInfo i;

    if (Tcl_GetCommandInfo(tcl, (char*)h, &i) && (i.proc == obj_handle))
        return ((Object*) i.clientData);

    return 0;
}

int Object::method(int, const char*[]) {
    TCL_Error(interp, "Object has no methods");
}

int obj_handle(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]) {
    Object* object = (Object*) c;

    assert(object->tcl() == tcl);
    return object->method(argc, argv);
}
