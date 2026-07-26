/* Copyright (c) 1993 by Sanjay Ghemawat */
#include <assert.h>
#include <string.h>
#include "basic.h"
#include "handle.h"

HandleTable::HandleTable() {
    Tcl_InitHashTable(&handleMap, TCL_STRING_KEYS);
    Tcl_InitHashTable(&objectMap, TCL_ONE_WORD_KEYS);
}

HandleTable::~HandleTable() {
    Tcl_DeleteHashTable(&objectMap);

    /* Delete handle name strings */
    Tcl_HashSearch search;
    Tcl_HashEntry* i;

    for (i = Tcl_FirstHashEntry(&handleMap, &search);
	 i != NULL;
	 i = Tcl_NextHashEntry(&search)) {
	delete [] Tcl_GetHashKey(&handleMap, i);
    }

    Tcl_DeleteHashTable(&handleMap);
}

void HandleTable::bind(char* handle, void* object) {
    /* Allocate string for handle name */
    char* copy = new char[strlen(handle)+1];
    strcpy(copy, handle);
    handle = copy;

    Tcl_HashEntry* entry;
    int newentry;

    /* Create handle->object binding */
    entry = Tcl_CreateHashEntry(&handleMap, handle, &newentry);
    assert(newentry);
    Tcl_SetHashValue(entry, object);

    /* Create object->handle binding */
    entry = Tcl_CreateHashEntry(&objectMap, (char*) object, &newentry);
    assert(newentry);
    Tcl_SetHashValue(entry, handle);
}

void* HandleTable::object(char* handle) {
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&handleMap, handle);
    if (entry != NULL) {
	return (void*) Tcl_GetHashValue(entry);
    }
    else {
	return 0;
    }
}

char* HandleTable::handle(void* object) {
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&objectMap, (char*) object);
    if (entry != NULL) {
	return (char*) Tcl_GetHashValue(entry);
    }
    else {
	return 0;
    }
}

void HandleTable::remove(char* handle, void* object) {
    Tcl_HashEntry* entry;

    entry = Tcl_FindHashEntry(&objectMap, (char*) object);
    if (entry != NULL) {
	Tcl_DeleteHashEntry(entry);
    }

    entry = Tcl_FindHashEntry(&handleMap, handle);
    if (entry != NULL) {
	delete [] Tcl_GetHashKey(&handleMap, entry);
	Tcl_DeleteHashEntry(entry);
    }
}
