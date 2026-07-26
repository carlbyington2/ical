/* Copyright (c) 1993 by Sanjay Ghemawat */

#include <assert.h>
#include <string.h>

#include "cal_tcl.h"
#include "collect.h"
#include "dispatch.h"
#include "ical.h"
#include "item_tcl.h"

#include "basic.h"
#include "Array.h"
#include "calendar.h"
#include "calfile.h"
#include "uid.h"

implementArray(FileList,CalFile*)

Calendar_Tcl::Calendar_Tcl(Tcl_Interp* tcl, char const* h, char const* f)
    : Object(tcl, "Calendar", h)
{
    main = new CalFile(0, f);
    includes = new FileList;

    /* Initialize error buffer */
    msg = new charArray(1000);
    clear_error();

    if (! main->Read()) {
        add_error(main->GetName(), CalFile::LastError());
        return;
    }
    add_item_handles(main);

    fix_includes();
}

Calendar_Tcl::~Calendar_Tcl() {
    for (int i = 0; i < includes->size(); i++) {
        CalFile* file = includes->slot(i);
        remove_item_handles(file->GetCalendar());
        delete file;
    }
    remove_item_handles(main->GetCalendar());

    delete includes;
    delete main;
    delete msg;
}

void Calendar_Tcl::add_item_handles(CalFile* cal) {
    int count = cal->GetCalendar()->Size();
    for (int i = 0; i < count; i++) {
        // the object gets automatically inserted into tcl hash table
        new Item_Tcl(tcl(), cal->GetCalendar()->Get(i), cal);
    }
}

void Calendar_Tcl::remove_item_handles(Calendar* cal) {
    int count = cal->Size();
    for (int i = 0; i < count; i++) {
        Item_Tcl* item = Item_Tcl::find(cal->Get(i));
        if (item != 0)
            delete item;
    }
}

CalFile* Calendar_Tcl::name2file(char const* name) {
    if (name == 0) {
        return main;
    }

    if (strcmp(name, main->GetName()) == 0) {
        return main;
    }

    for (int i = 0; i < includes->size(); i++) {
        /* Extra check for name2file use in fix_includes */
        if (includes->slot(i) == 0) {
            continue;
        }

        if (strcmp(includes->slot(i)->GetName(), name) == 0) {
            return includes->slot(i);
        }
    }
    return 0;
}

CalFile* Calendar_Tcl::cal2file(Calendar* c) {
    if (c == main->GetCalendar()) {
        return main;
    }

    for (int i = 0; i < includes->size(); i++) {
        if (includes->slot(i)->GetCalendar() == c) {
            return includes->slot(i);
        }
    }
    return 0;
}

void Calendar_Tcl::purge() {
    // Collect all active uids
    UidSet* elements = new UidSet;
    for (int i = 0; i <= includes->size(); i++) {
        // Iterate once more than necessary to scan the main calendar as well
        Calendar* calendar = ((i >= includes->size())
                              ? main
                              : includes->slot(i))->GetCalendar();

        for (int j = 0; j < calendar->Size(); j++) {
            elements->insert(calendar->Get(j)->GetUid());
        }
    }

    // Remove hide entries for all non-active items
    main->GetCalendar()->RestrictHidden(elements);
    main->Modified();
    delete elements;
}

void Calendar_Tcl::fix_includes() {
    /* Fixup include list */
    int i;
    FileList* new_includes = new FileList;

    for (i = 0; i < main->GetCalendar()->NumIncludes(); i++) {
        char const* name = main->GetCalendar()->GetInclude(i);

        /* Try to reuse file from old include list */
        CalFile* file = name2file(name);
        if (file == 0) {
            /* Create new file */
            file = new CalFile(0, name);
            if (! file->Read())
                add_error(file->GetName(), CalFile::LastError());
            add_item_handles(file);
        }
        else {
            /* Reuse old calendar - need to mark old include list */
            for (int j = 0; j < includes->size(); j++) {
                if (includes->slot(j) == file)
                    includes->slot(j) = 0;
            }
        }

        new_includes->append(file);
    }

    /* Remove old includes that were not reused */
    for (i = 0; i < includes->size(); i++) {
        CalFile* file = includes->slot(i);

        // Still included in main calendar?
        if (file == 0) continue;

        // Get rid of it
        if (file->IsModified()) {
            if (! file->Write())
                add_error(file->GetName(), CalFile::LastError());
        }

        remove_item_handles(file->GetCalendar());
        delete file;
    }

    delete includes;
    includes = new_includes;
}

void Calendar_Tcl::clear_error() {
    had_error = 0;
    msg->clear();
    msg->append('\0');
}

void Calendar_Tcl::add_error(char const* t1, char const* t2) {
    had_error = 1;

    /* Remove trailing null */
    msg->remove();

    msg->concat(t1, strlen(t1));
    msg->concat(": ", 2);
    msg->concat(t2, strlen(t2));
    msg->append('\n');
    msg->append('\0');
}

/*
 * Forward declaration of handler procedures.
 */
static int cal_delete   (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_main     (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_include  (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_exclude  (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_forincs  (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_add      (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_remove   (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_hide     (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_ronly    (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_dirty    (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_stale    (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_save     (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_reread   (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_query    (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_listing  (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_loop     (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_incal    (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_option   (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_doption  (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_loopf    (ClientData, Tcl_Interp*, int, const char*[]);
static int cal_loopb    (ClientData, Tcl_Interp*, int, const char*[]);

static Dispatch_Entry calendar_dispatch[] = {
    { "delete",         0, 0, cal_delete        },
    { "main",           0, 0, cal_main          },
    { "include",        1, 1, cal_include       },
    { "exclude",        1, 1, cal_exclude       },
    { "forincludes",    2, 2, cal_forincs       },
    { "add",            1, 2, cal_add           },
    { "remove",         1, 1, cal_remove        },
    { "hide",           1, 1, cal_hide          },
    { "readonly",       0, 1, cal_ronly         },
    { "dirty",          0, 1, cal_dirty         },
    { "stale",          0, 1, cal_stale         },
    { "save",           0, 1, cal_save          },
    { "reread",         0, 1, cal_reread        },
    { "query",          5, 7, cal_query         },
    { "listing",        5, 7, cal_listing       },
    { "loop",           2, 4, cal_loop          },
    { "incalendar",     3, 3, cal_incal         },
    { "option",         1, 4, cal_option        },
    { "delete_option",  1, 3, cal_doption       },
    { "loop_forward",   5, 7, cal_loopf         },
    { "loop_backward",  5, 7, cal_loopb         },
    { 0,                0, 0, 0                 }
};

// Helper routine for parsing set of items that will be covered by
// query operations.

static int parse_items(Tcl_Interp* tcl, Calendar_Tcl* cal, ItemList& items,
                       int& argc, const char**& argv)
{
    // modifies items, argc, argv
    // effects  If "argc/argv" starts with "-all", then strips off
    //          that option and appends all items from all calendars to "items".
    //          If "argc/argv" starts with "-calendar <calname>", then
    //          strips off that option and appends all items from named
    //          calendar to "items".
    //          If "argc/argv" starts with "-items <list of items>", then
    //          strips off that option and appends all listed items
    //          to "items".
    //          Otherwise, does not modify "argc/argv" and appends all
    //          items from all visible calendars to "items".
    //
    //          Normally returns TCL_OK, but if there is an error parsing
    //          the calendar name or the list of items, then sets the
    //          TCL return value to an error message and returns TCL_ERROR.

    if ((argc >= 1) && (strcmp(argv[0], "-all") == 0)) {
        collect_all(cal, items, false);
        argc--;
        argv++;
        return TCL_OK;
    }

    if ((argc >= 2) && (strcmp(argv[0], "-calendar") == 0)) {
        CalFile* file = cal->name2file(argv[1]);
        if (file == 0) {
            TCL_Error(tcl, "no such calendar");
        }

        collect_calendar(cal, file->GetCalendar(), items);
        argc -= 2;
        argv += 2;
        return TCL_OK;
    }

    if ((argc >= 2) && (strcmp(argv[0], "-items") == 0)) {
        int count;
        const char** list;
        if (Tcl_SplitList(tcl, argv[1], &count, &list) != TCL_OK) {
            TCL_Error(tcl, "invalid item list");
        }

        // Stash initial array size so we can abort on error
        int isize = items.size();
        for (int i = 0; i < count; i++) {
            Object* obj = Object::find(tcl, list[i]);
            if ((obj == 0) || (strcmp(obj->type(), "Item") != 0)) {
                Tcl_Free((char*) list);
                items.remove(items.size() - isize);
                TCL_Error(tcl, "no such item");
            }
            items.append((Item_Tcl*) obj);
        }

        argc -= 2;
        argv += 2;
        return TCL_OK;
    }

    // Default behavior is to return all items
    collect_all(cal, items, true);
    return TCL_OK;
}

int Calendar_Tcl::method(int argc, const char* argv[]) {
    return Dispatch(calendar_dispatch, (ClientData)this, tcl(), argc, argv);
}

static int cal_delete(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;
    delete cal;

    TCL_Return(tcl, "");
}

static int cal_main(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;
    TCL_Return(tcl, (char*) cal->main->GetName());
}

static int cal_include(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    if (cal->main->GetCalendar()->ReadOnly()) {
        TCL_Error(tcl, "permission denied");
    }

    CalFile* newFile = new CalFile(0, argv[0]);
    if (! newFile->Read()) {
        delete newFile;
        TCL_Error(tcl, (char*) CalFile::LastError());
    }
    cal->add_item_handles(newFile);

    cal->main->GetCalendar()->Include(argv[0]);
    cal->includes->append(newFile);
    cal->main->Modified();

    trigger(tcl, "flush", 0);

    TCL_Return(tcl, "");
}

static int cal_exclude(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    if (cal->main->GetCalendar()->ReadOnly()) {
        TCL_Error(tcl, "permission denied");
    }

    for (int i = 0; i < cal->includes->size(); i++) {
        CalFile* f = cal->includes->slot(i);
        if (strcmp(f->GetName(), argv[0]) == 0) {
            if (f->IsModified()) {
                TCL_Error(tcl, "cannot exclude dirty calendar");
            }

            /* Remove it */
            cal->remove_item_handles(f->GetCalendar());
            delete f;

            cal->main->GetCalendar()->Exclude(argv[0]);
            for (int j = i+1; j < cal->includes->size(); j++)
                cal->includes->slot(j-1) = cal->includes->slot(j);
            cal->includes->remove();
            cal->main->Modified();

            trigger(tcl, "flush", 0);

            TCL_Return(tcl, "");
        }
    }

    TCL_Error(tcl, "no such calendar");
}

static int cal_forincs(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    const char* var = argv[0];
    const char* body = argv[1];
    for (int i = 0; i < cal->includes->size(); i++) {
        if (Tcl_SetVar(tcl, var, (char*) (cal->includes->slot(i)->GetName()),
                       0) == NULL) {
            TCL_Error(tcl, "could not set loop variable");
        }

        int result = Tcl_Eval(tcl, body);
        if (result == TCL_BREAK) break;
        if ((result == TCL_OK) || (result == TCL_CONTINUE)) continue;
        return result;
    }

    TCL_Return(tcl, "");
}

static int cal_add(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    // Find item
    Object* obj = Object::find(tcl, argv[0]);
    if ((obj == 0) || (strcmp(obj->type(), "Item") != 0)) {
        TCL_Error(tcl, "no such item");
    }
    Item_Tcl* item = (Item_Tcl*) obj;

    CalFile* file = cal->name2file(argv[1]);
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    // Permission checks on old and new calendars
    CalFile* old = item->calendar();
    if ((old != 0) && old->GetCalendar()->ReadOnly()) {
        TCL_Error(tcl, "permission denied");
    }

    if (file->GetCalendar()->ReadOnly()) {
        TCL_Error(tcl, "permission denied");
    }

    // Mark item as moved
    item->set_calendar(file);

    // Remove from old calendar
    if (old != 0) {
        old->GetCalendar()->Remove(item->value());
        old->Modified();
    }

    // Put in new calendar
    file->GetCalendar()->Add(item->value());
    file->Modified();

    trigger(tcl, ((old == 0) ? "add" : "change"), item->handle());

    TCL_Return(tcl, "");
}

static int cal_remove(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    // Find item
    Object* obj = Object::find(tcl, argv[0]);
    if ((obj == 0) || (strcmp(obj->type(), "Item") != 0)) {
        TCL_Error(tcl, "no such item");
    }
    Item_Tcl* item = (Item_Tcl*) obj;

    // Find file
    CalFile* file = item->calendar();
    if (file == 0) TCL_Error(tcl, "no such calendar");

    if (file->GetCalendar()->ReadOnly()) {
        TCL_Error(tcl, "permission denied");
    }

    item->set_calendar(0);
    file->GetCalendar()->Remove(item->value());
    file->Modified();

    trigger(tcl, "delete", item->handle());

    TCL_Return(tcl, "");
}

static int cal_hide(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    // Find item
    Object* obj = Object::find(tcl, argv[0]);
    if ((obj == 0) || (strcmp(obj->type(), "Item") != 0)) {
        TCL_Error(tcl, "no such item");
    }
    Item_Tcl* item = (Item_Tcl*) obj;

    CalFile* file = item->calendar();
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    CalFile* mainFile = cal->main;

    if (mainFile->GetCalendar()->ReadOnly()) {
        TCL_Error(tcl, "permission denied");
    }
    mainFile->GetCalendar()->Hide(item->value()->GetUid());
    mainFile->Modified();

    // The hide entry in the main calendar depends on the uid
    // of the item being hidden.  Therefore we need to make sure
    // that the uid gets written out in case it was just
    // assigned.

    if (!item->value()->IsUidPersistent() &&
        !file->GetCalendar()->ReadOnly()) {
        file->Modified();
    }

    trigger(tcl, "delete", item->handle());

    TCL_Return(tcl, "");
}

static int cal_ronly(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    CalFile* file = cal->name2file(argv[0]);
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    TCL_Return(tcl, file->GetCalendar()->ReadOnly() ? "1" : "0");
}

static int cal_dirty(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    CalFile* file = cal->name2file(argv[0]);
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    TCL_Return(tcl, (file->IsModified() ? "1" : "0"));
}

static int cal_stale(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    CalFile* file = cal->name2file(argv[0]);
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    TCL_Return(tcl, (file->FileHasChanged() ? "1" : "0"));
}

static int cal_save(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    CalFile* file = cal->name2file(argv[0]);
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    if (file == cal->main) {
        // Purge unnecessary hidden entries.
        cal->purge();
    }

    if (! file->Write()) {
        TCL_Error(tcl, (char*) CalFile::LastError());
    }
    TCL_Return(tcl, "");
}

static int cal_reread(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    CalFile* file = cal->name2file(argv[0]);
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    Calendar* old = file->ReRead();
    if (old == 0) {
        TCL_Error(tcl, (char*) CalFile::LastError());
    }

    /* Fixup items */
    cal->remove_item_handles(old);
    cal->add_item_handles(file);
    delete old;

    if (file == cal->main) {
        cal->clear_error();
        cal->fix_includes();
        /* XXX Report error??? */
    }

    trigger(tcl, "flush", 0);

    TCL_Return(tcl, "");
}

// Common routine for looping over itesm
//
// effects      For each "item" in "list",
//                  set "ivar" to "item" handle
//                  set "dvar" to corresponding date if "dvar" is not null
//                  execute "body" within "tcl".
//              If executions succeed, clear "tcl->result" and return TCL_OK.
//              If executions fail, set "tcl->result" and return TCL_ERROR.
//              If "body" says "break", interrupt the iteration.
//              If "body" says "continue", move to the next iteration

static int item_loop(Tcl_Interp* tcl, Occurrences const& list,
                     const char* ivar, const char* dvar, const char* body) {
    for (int i = 0; i < list.size(); i++) {
        if (Tcl_SetVar(tcl, ivar, (char*) list[i].item->handle(), 0) == NULL) {
            TCL_Error(tcl, "could not set loop variable");
        }

        if (dvar != 0) {
            char buffer[20];
            sprintf(buffer, "%d", list[i].date.EpochDays());
            if (Tcl_SetVar(tcl, dvar, buffer, 0) == NULL) {
                TCL_Error(tcl, "could not set loop variable");
            }
        }

        int result = Tcl_Eval(tcl, body);
        if (result == TCL_BREAK) break;
        if ((result == TCL_OK) || (result == TCL_CONTINUE)) continue;
        return result;
    }
    TCL_Return(tcl, "");
}

static int cal_query(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    ItemList items;
    if (parse_items(tcl, cal, items, argc, argv) != TCL_OK) return TCL_ERROR;
    if (argc < 5) TCL_Error(tcl, "not enough arguments");
    if (argc > 5) TCL_Error(tcl, "too many arguments");

    int startDays;
    if (Tcl_GetInt(tcl, argv[0], &startDays) != TCL_OK) {
        TCL_Error(tcl, "illegal start date");
    }
    Date start(startDays);

    int finishDays;
    if (Tcl_GetInt(tcl, argv[1], &finishDays) != TCL_OK) {
        TCL_Error(tcl, "illegal finish date");
    }
    Date finish(finishDays);

    Occurrences list;
    collect_occurrences(cal, items, list, start, finish, 0);
    sort_occurrences(list);

    return (item_loop(tcl, list, argv[2], argv[3], argv[4]));
}

static int cal_loopf(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    ItemList items;
    if (parse_items(tcl, cal, items, argc, argv) != TCL_OK) return TCL_ERROR;
    if (argc < 5) TCL_Error(tcl, "not enough arguments");
    if (argc > 5) TCL_Error(tcl, "too many arguments");

    // Find starting item
    Item_Tcl* item = 0;
    if (strcmp(argv[0], "") != 0) {
        Object* obj = Object::find(tcl, argv[0]);
        if ((obj == 0) || (strcmp(obj->type(), "Item") != 0)) {
            TCL_Error(tcl, "no such item");
        }
        item = (Item_Tcl*) obj;
    }

    int startDays;
    if (Tcl_GetInt(tcl, argv[1], &startDays) != TCL_OK) {
        TCL_Error(tcl, "illegal start date");
    }
    Date start(startDays);
    Date finish = Date::Last();
    Occurrences list;

    // Special case handling if only some of the items from the first
    // date should be returned.
    if (item != 0) {
        Occurrences tmp;
        collect_occurrences(cal, items, tmp, start, start, 0);
        sort_occurrences(tmp);

        int i = 0;
        while ((i < tmp.size()) && (tmp[i].item != item))
            i++;
        i++;
        while (i < tmp.size()) {
            list.append(tmp[i]);
            i++;
        }

        int result = item_loop(tcl, list, argv[2], argv[3], argv[4]);
        if ((result != TCL_OK) && (result != TCL_CONTINUE)) return result;

        start = start + 1;
    }

    Date date = start;
    int days = 1;
    while (date <= finish) {
        Date limit = (((finish-date) + 1) < days) ? finish : (date+days - 1);
        collect_occurrences(cal, items, list, date, limit, 0);
        sort_occurrences(list);
        int result = item_loop(tcl, list, argv[2], argv[3], argv[4]);
        if ((result != TCL_OK) && (result != TCL_CONTINUE)) return result;

        date = date + days;
        if (list.size() == 0) {
            // No items found in last "days".  Increase searching span.
            days *= 2;
        }
    }

    TCL_Return(tcl, "");
}

static int cal_loopb(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    ItemList items;
    if (parse_items(tcl, cal, items, argc, argv) != TCL_OK) return TCL_ERROR;
    if (argc < 5) TCL_Error(tcl, "not enough arguments");
    if (argc > 5) TCL_Error(tcl, "too many arguments");

    // Find starting item
    Item_Tcl* item = 0;
    if (strcmp(argv[0], "") != 0) {
        Object* obj = Object::find(tcl, argv[0]);
        if ((obj == 0) || (strcmp(obj->type(), "Item") != 0)) {
            TCL_Error(tcl, "no such item");
        }
        item = (Item_Tcl*) obj;
    }

    int startDays;
    if (Tcl_GetInt(tcl, argv[1], &startDays) != TCL_OK) {
        TCL_Error(tcl, "illegal start date");
    }
    Date start(startDays);
    Date finish = Date::First();
    Occurrences list;

    // Special case handling if only some of the items from the first
    // date should be returned.
    if (item != 0) {
        Occurrences tmp;
        collect_occurrences(cal, items, tmp, start, start, 0);
        sort_occurrences(tmp);

        int i = 0;
        while ((i < tmp.size()) && (tmp[i].item != item)) {
            list.append(tmp[i]);
            i++;
        }
        reverse(list);

        int result = item_loop(tcl, list, argv[2], argv[3], argv[4]);
        if ((result != TCL_OK) && (result != TCL_CONTINUE)) return result;

        start = start - 1;
    }

    Date date = start;
    int days = 1;
    while (date >= finish) {
        Date limit = (((date-finish) + 1) < days) ? finish : (date + 1 - days);
        collect_occurrences(cal, items, list, limit, date, 0);
        sort_occurrences(list);
        reverse(list);

        int result = item_loop(tcl, list, argv[2], argv[3], argv[4]);
        if ((result != TCL_OK) && (result != TCL_CONTINUE)) return result;

        date = date - days;
        if (list.size() == 0) {
            // No items found in last "days".  Increase searching span.
            days *= 2;
        }
    }

    TCL_Return(tcl, "");
}

static int cal_listing(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    ItemList items;
    if (parse_items(tcl, cal, items, argc, argv) != TCL_OK) return TCL_ERROR;
    if (argc < 5) TCL_Error(tcl, "not enough arguments");
    if (argc > 5) TCL_Error(tcl, "too many arguments");

    int startDays;
    if (Tcl_GetInt(tcl, argv[0], &startDays) != TCL_OK) {
        TCL_Error(tcl, "illegal start date");
    }
    Date start(startDays);

    int finishDays;
    if (Tcl_GetInt(tcl, argv[1], &finishDays) != TCL_OK) {
        TCL_Error(tcl, "illegal finish date");
    }
    Date finish(finishDays);

    Occurrences list;
    collect_occurrences(cal, items, list, start, finish, 1);
    sort_occurrences(list);

    return (item_loop(tcl, list, argv[2], argv[3], argv[4]));
}

static int cal_loop(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    ItemList items;
    if (parse_items(tcl, cal, items, argc, argv) != TCL_OK) return TCL_ERROR;
    if (argc < 2) TCL_Error(tcl, "not enough arguments");
    if (argc > 2) TCL_Error(tcl, "too many arguments");

    // Generate first occurrence of each item
    Occurrences list;
    for (int i = 0; i < items.size(); i++) {
        Item_Tcl* item = items[i];

        Date d;
        if (item->value()->first(d)) {
            Occurrence o;
            o.item = item;
            o.date = d;
            list.append(o);
        }
    }

    sort_occurrences(list);
    return (item_loop(tcl, list, argv[0], 0, argv[1]));
}

static int cal_incal(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]){
    Calendar_Tcl* cal = (Calendar_Tcl*) c;

    CalFile* file = cal->name2file(argv[0]);
    if (file == 0) {
        TCL_Error(tcl, "no such calendar");
    }

    /* Collect items */
    ItemList items;
    collect_calendar(cal, file->GetCalendar(), items);

    // Generate first occurrence of each item
    Occurrences list;
    for (int i = 0; i < items.size(); i++) {
        Item_Tcl* item = items[i];

        Date d;
        if (item->value()->first(d)) {
            Occurrence o;
            o.item = item;
            o.date = d;
            list.append(o);
        }
    }

    sort_occurrences(list);
    return (item_loop(tcl, list, argv[1], 0, argv[2]));
}

static int cal_option(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]) {
    Calendar_Tcl* cal = (Calendar_Tcl*) c;
    CalFile* file = cal->main;

    // See if a specific calendar is mentioned.
    if ((argc > 2) && (strcmp(argv[0], "-calendar") == 0)) {
        file = cal->name2file(argv[1]);
        if (file == 0) TCL_Error(tcl, "no such calendar");
        argv += 2;
        argc -= 2;
    }

    if (argc > 2) TCL_Error(tcl, "too many arguments");

    if (argc == 1) {
        char const* val = file->GetCalendar()->GetOption(argv[0]);
        if (val != 0) {
            Tcl_SetResult(tcl, (char*)val, TCL_VOLATILE);
            return TCL_OK;
        }
        TCL_Error(tcl, "unknown calendar option");
    }

    if (file->GetCalendar()->ReadOnly())
        TCL_Error(tcl, "permission denied");

    file->GetCalendar()->SetOption(argv[0], argv[1]);
    file->Modified();
    TCL_Return(tcl, "");
}

static int cal_doption(ClientData c, Tcl_Interp* tcl, int argc, const char* argv[]) {
    Calendar_Tcl* cal = (Calendar_Tcl*) c;
    CalFile* file = cal->main;

    // See if a specific calendar is mentioned.
    if ((argc > 1) && (strcmp(argv[0], "-calendar") == 0)) {
        file = cal->name2file(argv[1]);
        if (file == 0) TCL_Error(tcl, "no such calendar");
        argv += 2;
        argc -= 2;
    }
    if (argc > 1) TCL_Error(tcl, "too many arguments");

    if (file->GetCalendar()->ReadOnly()) TCL_Error(tcl, "permission denied");

    // Check that option exists
    char const* val = file->GetCalendar()->GetOption(argv[0]);
    if (val == 0) TCL_Error(tcl, "unknown calendar option");

    file->GetCalendar()->RemoveOption(argv[0]);
    file->Modified();
    TCL_Return(tcl, "");
}
