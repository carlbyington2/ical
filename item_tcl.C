/* Copyright (c) 1993 by Sanjay Ghemawat */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "WeekDay.h"

#include "basic.h"
#include "arrays.h"
#include "cal_tcl.h"
#include "calfile.h"
#include "calendar.h"
#include "ical.h"
#include "item_tcl.h"
#include "dispatch.h"

/*
 * Item* -> Item_Tcl* map.
 */
static Tcl_HashTable itemMap;
static int itemMapInited = 0;

static int check_permission(Tcl_Interp* tcl, Item_Tcl* item);
// effects      If "item" can be modified legally, then mark
//              containing calendar as modified and return true.
//              Else store an error message in "tcl->result"
//              and return false.

static void trigger_item(Tcl_Interp* tcl, Item_Tcl* item,
                         char const* = "change");
// effects      If "item" belongs to a calendar, generate a trigger message
//              for it.  The generate trigger is of type "change" by default,
//              but that can be overridden by providing a different value for
//              the optional last argument.

Item_Tcl::Item_Tcl(Tcl_Interp* tcl, Item* i, CalFile* c)
    : Object(tcl, "Item")
{
    item = i;
    cal = c;

    if (! itemMapInited) {
        itemMapInited = 1;
        Tcl_InitHashTable(&itemMap, TCL_ONE_WORD_KEYS);
    }
    int newentry;
    Tcl_HashEntry* entry = Tcl_CreateHashEntry(&itemMap,(char*)item,&newentry);
    assert(newentry);
    Tcl_SetHashValue(entry, (ClientData)this);
}

Item_Tcl::~Item_Tcl() {
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&itemMap, (char*)item);
    assert((entry != 0) && (Tcl_GetHashValue(entry) == ((ClientData) this)));
    Tcl_DeleteHashEntry(entry);
}

Item_Tcl* Item_Tcl::find(Item* item) {
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&itemMap, (char*)item);
    return ((entry == 0) ? 0 : ((Item_Tcl*) Tcl_GetHashValue(entry)));
}

/*
 * Forward declaration of handler procedures.
 */
static int item_length    (ClientData, Tcl_Interp*, int, const char**);
static int item_starttime (ClientData, Tcl_Interp*, int, const char**);
static int item_timezone  (ClientData, Tcl_Interp*, int, const char**);
static int item_clone     (ClientData, Tcl_Interp*, int, const char**);
static int item_is        (ClientData, Tcl_Interp*, int, const char**);
static int item_delete    (ClientData, Tcl_Interp*, int, const char**);
static int item_cal       (ClientData, Tcl_Interp*, int, const char**);
static int item_text      (ClientData, Tcl_Interp*, int, const char**);
static int item_uid       (ClientData, Tcl_Interp*, int, const char**);
static int item_early     (ClientData, Tcl_Interp*, int, const char**);
static int item_owner     (ClientData, Tcl_Interp*, int, const char**);
static int item_owned     (ClientData, Tcl_Interp*, int, const char**);
static int item_own       (ClientData, Tcl_Interp*, int, const char**);
static int item_hilite    (ClientData, Tcl_Interp*, int, const char**);
static int item_todo      (ClientData, Tcl_Interp*, int, const char**);
static int item_is_done   (ClientData, Tcl_Interp*, int, const char**);
static int item_done      (ClientData, Tcl_Interp*, int, const char**);
static int item_alarms    (ClientData, Tcl_Interp*, int, const char**);
static int item_option    (ClientData, Tcl_Interp*, int, const char**);
static int item_doption   (ClientData, Tcl_Interp*, int, const char**);
static int item_empty     (ClientData, Tcl_Interp*, int, const char**);
static int item_repeat    (ClientData, Tcl_Interp*, int, const char**);
static int item_first     (ClientData, Tcl_Interp*, int, const char**);
static int item_type      (ClientData, Tcl_Interp*, int, const char**);
static int item_desc      (ClientData, Tcl_Interp*, int, const char**);
static int item_cont      (ClientData, Tcl_Interp*, int, const char**);
static int item_next      (ClientData, Tcl_Interp*, int, const char**);
static int item_range     (ClientData, Tcl_Interp*, int, const char**);
static int item_date      (ClientData, Tcl_Interp*, int, const char**);
static int item_start     (ClientData, Tcl_Interp*, int, const char**);
static int item_finish    (ClientData, Tcl_Interp*, int, const char**);
static int item_ondel     (ClientData, Tcl_Interp*, int, const char**);
static int item_dayr      (ClientData, Tcl_Interp*, int, const char**);
static int item_wdays     (ClientData, Tcl_Interp*, int, const char**);

static int item_monthr    (ClientData, Tcl_Interp*, int, const char**);
static int item_mday      (ClientData, Tcl_Interp*, int, const char**);
static int item_mlday     (ClientData, Tcl_Interp*, int, const char**);
static int item_mworkday  (ClientData, Tcl_Interp*, int, const char**);
static int item_mlworkday (ClientData, Tcl_Interp*, int, const char**);
static int item_mweekday  (ClientData, Tcl_Interp*, int, const char**);
static int item_mlweekday (ClientData, Tcl_Interp*, int, const char**);

static Dispatch_Entry item_dispatch[] = {
    { "delete",                 0, 0, item_delete       },
    { "clone",                  0, 0, item_clone        },

    { "length",                 0, 1, item_length       },
    { "starttime",              1, 2, item_starttime    },
    { "timezone",               0, 2, item_timezone     },
    { "alarms",                 0, 1, item_alarms       },
    { "option",                 1, 2, item_option       },
    { "delete_option",          1, 1, item_doption      },

    { "is",                     1, 1, item_is           },
    { "calendar",               0, 0, item_cal          },
    { "text",                   0, 1, item_text         },
    { "uid",                    0, 0, item_uid          },
    { "earlywarning",           0, 1, item_early        },
    { "owner",                  0, 1, item_owner        },
    { "owned",                  0, 0, item_owned        },
    { "own",                    0, 0, item_own          },
    { "hilite",                 0, 1, item_hilite       },
    { "todo",                   0, 1, item_todo         },
    { "is_done",                0, 0, item_is_done      },
    { "done",                   1, 1, item_done         },

    { "contains",               1, 1, item_cont         },
    { "empty",                  0, 0, item_empty        },
    { "repeats",                0, 0, item_repeat       },
    { "first",                  0, 0, item_first        },
    { "next",                   1, 1, item_next         },
    { "range",                  2, 3, item_range        },
    { "type",                   0, 0, item_type         },
    { "describe_repeat",        0, 1, item_desc         },

    { "date",                   1, 1, item_date         },
    { "dayrepeat",              2, 2, item_dayr         },

    { "monthrepeat",            2, 2, item_monthr       },
    { "month_day",              1, 3, item_mday         },
    { "month_last_day",         1, 3, item_mlday        },
    { "month_work_day",         1, 3, item_mworkday     },
    { "month_last_work_day",    1, 3, item_mlworkday    },
    { "month_week_day",         2, 4, item_mweekday     },
    { "month_last_week_day",    2, 4, item_mlweekday    },

    { "weekdays",               0, -1, item_wdays       },
    { "start",                  1,  2, item_start       },
    { "finish",                 1,  2, item_finish      },
    { "deleteon",               1,  1, item_ondel       },

    { 0,                        0, 0, 0                 }
};

int Item_Tcl::method(int argc, const char** argv) {
    return Dispatch(item_dispatch, (ClientData)this, tcl(), argc, argv);
}

/*
 * Handler procedures.
 */

static int item_length(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    Appointment* appt = item->value()->AsAppointment();
    if (appt != 0) {
        if (argc == 0) {
            char buffer[100];
            sprintf(buffer, "%d", appt->GetLength());
            Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
            return TCL_OK;
        }

        int length;
        if ((Tcl_GetInt(tcl, argv[0], &length) != TCL_OK) ||
            (length < 1) ||
            (length > 24*60)) {
            TCL_Error(tcl, "invalid appointment length");
        }
        if (! check_permission(tcl, item)) return TCL_ERROR;

        appt->SetLength(length);
        trigger_item(tcl, item);

        TCL_Return(tcl, "");
    }

    TCL_Error(tcl, "unknown command");
}

static int item_starttime(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    int date;

    Appointment* appt = item->value()->AsAppointment();
    if (appt == 0) {
        TCL_Error(tcl, "unknown command");
    }

    if (strcmp(argv[0], "native") == 0) {
        date = -1;
    } else {
        if (Tcl_GetInt(tcl, argv[0], &date) != TCL_OK || date < 0) {
            TCL_Error(tcl, "invalid starttime's date");
        }
    }

    if (argc == 1) {
        char buffer[100];
        sprintf(buffer, "%d", appt->GetStart(date));
        Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
        return TCL_OK;
    }

    int start;
    if ((Tcl_GetInt(tcl, argv[1], &start) != TCL_OK) ||
        (start < 0) || (start >= (24*60))) {
        TCL_Error(tcl, "invalid appointment start");
    }
    if (! check_permission(tcl, item)) return TCL_ERROR;

    appt->SetStart(date, start);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_timezone(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    bool convert_remote=false;

    Appointment* appt = item->value()->AsAppointment();
    if (appt == 0) {
        TCL_Error(tcl, "unknown command");
    }

    if (argc == 0) {
        TCL_Return(tcl, (char*) appt->GetTimezone());
    }

    if (argc > 1 && strcmp(argv[0], "-convert") == 0) {
        convert_remote=true;
        argc--;
        argv++;
    }

    if (! check_permission(tcl, item)) return TCL_ERROR;
    appt->SetTimezone(argv[0], convert_remote);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_clone(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    Item_Tcl* clone = new Item_Tcl(tcl, item->value()->Clone(), 0);
    TCL_Return(tcl, (char*) clone->handle());
}

static int item_is(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    if ((strcmp(argv[0], "note") == 0) && (item->value()->AsNotice() != 0)) {
        TCL_Return(tcl, "1");
    }

    if ((strcmp(argv[0], "appt") == 0) && (item->value()->AsAppointment()!=0)){
        TCL_Return(tcl, "1");
    }

    TCL_Return(tcl, "0");
}

static int item_delete(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = ((Item_Tcl*) c);
    if (! check_permission(tcl, item)) return TCL_ERROR;

    CalFile* file = item->calendar();
    if (file != 0) {
        item->set_calendar(0);
        file->GetCalendar()->Remove(item->value());
        file->Modified();

        // XXX Only send triggers when deleting an item from a calendar???
        trigger(tcl, "delete", item->handle());
    }

    delete item->value();
    delete item;

    TCL_Return(tcl, "");
}

static int item_cal(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    CalFile* cal = item->calendar();
    if (cal == 0) {
        TCL_Error(tcl, "item not in calendar");
    }
    TCL_Return(tcl, (char*)(cal->GetName()));
}

static int item_text(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    if (argc == 0) {
        TCL_Return(tcl, (char*) item->value()->GetText());
    }

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->SetText(argv[0]);
    trigger_item(tcl, item, "text");

    TCL_Return(tcl, "");
}

static int item_uid(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    TCL_Return(tcl, (char*) item->value()->GetUid());
}

static int item_early(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    if (argc == 0) {
        char buffer[100];

        sprintf(buffer, "%d", item->value()->GetRemindStart());
        Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
        return TCL_OK;
    }

    int warn;
    if (Tcl_GetInt(tcl, argv[0], &warn) != TCL_OK) {
        TCL_Error(tcl, "invalid early warning option");
    }
    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->SetRemindStart(warn);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_owner(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    if (argc == 0) {
        TCL_Return(tcl, (char*) item->value()->GetOwner());
    }

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->SetOwner(argv[0]);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_owned(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    TCL_Return(tcl, (item->value()->IsMine() ? "1" : "0"));
}

static int item_own(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->MakeOwner();
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_hilite(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    if (argc == 0) {
        Tcl_SetResult(tcl, (char*) (item->value()->Hilite()), TCL_VOLATILE);
        return TCL_OK;
    }

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->Hilite(argv[0]);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_todo(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    if (argc == 0) {
        TCL_Return(tcl, (char*)(item->value()->IsTodo() ? "1" : "0"));
    }

    int todo;
    if (Tcl_GetBoolean(tcl, argv[0], &todo) != TCL_OK) {
        TCL_Error(tcl, "invalid value for todo flag");
    }
    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->SetTodo(todo);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_is_done(ClientData c, Tcl_Interp* tcl,int argc,const char* argv[]) {
    Item_Tcl* item = (Item_Tcl*) c;
    TCL_Return(tcl, (char*)(item->value()->IsDone() ? "1" : "0"));
}

static int item_done(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    if (! check_permission(tcl, item)) return TCL_ERROR;

    int done;
    if (Tcl_GetBoolean(tcl, argv[0], &done) != TCL_OK) {
        TCL_Error(tcl, "invalid value for done flag");
    }

    item->value()->SetDone(done);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_alarms(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    Appointment* appt = item->value()->AsAppointment();
    if (appt == 0) {
        TCL_Error(tcl, "unknown command");
    }

    if (argc == 0) {
        intArray* alarms = appt->GetAlarms();
        if (alarms == 0) {
            TCL_Error(tcl, "no alarms");
        }

        // Make Tcl string out of integers
        // (We allocate an array one larger than necessary to avoid
        // zero length allocation).

        int i;
        char** str = new char*[alarms->size()+1];
        for (i = 0; i < alarms->size(); i++) {
            int x = alarms->slot(i);

            str[i] = new char[100];
            sprintf(str[i], "%d", x);
        }
        char* list = Tcl_Merge(alarms->size(), str);
        for (i = 0; i < alarms->size(); i++) {
            delete [] str[i];
        }
        delete [] str;

        Tcl_SetResult(tcl, list, TCL_DYNAMIC);
        return TCL_OK;
    }

    int count;
    const char** list;
    if (Tcl_SplitList(tcl, argv[0], &count, &list) != TCL_OK) {
        TCL_Error(tcl, "invalid alarm list");
    }

    intArray* alarms = new intArray(count);
    for (int i = 0; i < count; i++) {
        int x;
        if ((Tcl_GetInt(tcl, list[i], &x) != TCL_OK) || (x < 0) || (x > 60)) {
            Tcl_Free((char*) list);
            delete alarms;
            TCL_Error(tcl, "invalid alarm time");
        }
        alarms->append(x);
    }
    Tcl_Free((char*) list);
            
    if (! check_permission(tcl, item)) {
        delete alarms;
        return TCL_ERROR;
    }

    appt->SetAlarms(alarms);
    delete alarms;
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_option(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    if (argc == 1) {
        char const* val = item->value()->GetOption(argv[0]);
        if (val == 0) TCL_Error(tcl, "unknown item option");
        Tcl_SetResult(tcl, (char*) val, TCL_VOLATILE);
        return TCL_OK;
    }

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->SetOption(argv[0], argv[1]);
    trigger_item(tcl, item);
    TCL_Return(tcl, "");
}

static int item_doption(ClientData c, Tcl_Interp* tcl, int argc,const char* argv[]) {
    Item_Tcl* item = (Item_Tcl*) c;

    if (! check_permission(tcl, item)) return TCL_ERROR;

    // Check whether option exists
    char const* val = item->value()->GetOption(argv[0]);
    if (val == 0) TCL_Error(tcl, "unknown item option");

    item->value()->RemoveOption(argv[0]);
    trigger_item(tcl, item);
    TCL_Return(tcl, "");
}

static int item_empty(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    TCL_Return(tcl, (item->value()->empty()?"1":"0"));
}

static int item_repeat(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    TCL_Return(tcl,(item->value()->repeats()?"1":"0"));
}

static int item_first(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    Date d;
    if (! item->value()->first(d)) {
        TCL_Error(tcl, "item does not occur");
    }

    char buffer[100];
    sprintf(buffer, "%d", d.EpochDays());
    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
    return TCL_OK;
}

static int item_type(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    const char* result;
    switch (item->value()->repeat_type()) {
      case DateSet::None:
        result = "";
        break;
      case DateSet::Daily:
        result = "Daily";
        break;
      case DateSet::Weekly:
        result = "Weekly";
        break;
      case DateSet::BiWeekly:
        result = "Every Two Weeks";
        break;
      case DateSet::ThreeWeekly:
        result = "Every Three Weeks";
        break;
      case DateSet::FourWeekly:
        result = "Every Four Weeks";
        break;
      case DateSet::Monthly:
        result = "Monthly";
        break;
      case DateSet::TwoMonthly:
        result = "Every Two Months";
        break;
      case DateSet::ThreeMonthly:
        result = "Every Three Months";
        break;
      case DateSet::FourMonthly:
        result = "Every Four Months";
        break;
      case DateSet::SixMonthly:
        result = "Every Six Months";
        break;
      case DateSet::Annual:
        result = "Annual";
        break;
      default:
        result = "Complex";
        break;
    }
    TCL_Return(tcl, result);
}

static int item_desc(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    charArray buffer;

    if (argc == 0)
        item->value()->describe(&buffer);
    else
      if (argc == 1 && strcmp(argv[0], "-terse") == 0)
        item->value()->describe_terse(&buffer);
      else
        TCL_Error(tcl, "invalid argument(s)");
    buffer.append('\0');
    Tcl_SetResult(tcl, buffer.as_pointer(), TCL_VOLATILE);
    return TCL_OK;
}

static int item_cont(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    int dateDays;
    if (Tcl_GetInt(tcl, argv[0], &dateDays) != TCL_OK) {
        TCL_Error(tcl, "invalid date");
    }
    Date date(dateDays);
    TCL_Return(tcl, (item->value()->contains(date)?"1":"0"));
}

static int item_next(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    int dateDays;
    if (Tcl_GetInt(tcl, argv[0], &dateDays) != TCL_OK) {
        TCL_Error(tcl, "invalid date");
    }
    Date date(dateDays);
    Date next;
    if (! item->value()->next(date, next)) {
        TCL_Error(tcl, "no next occurrence for item");
    }

    char buffer[100];
    sprintf(buffer, "%d", next.EpochDays());
    Tcl_SetResult(tcl, buffer, TCL_VOLATILE);
    return TCL_OK;
}

static int item_range(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    Date s, f;
    if (item->value()->range(s, f)) {
        char buffer[100];
        sprintf(buffer, "%d", s.EpochDays());
        if (Tcl_SetVar(tcl, argv[0], buffer, 0) == NULL)
            TCL_Error(tcl, "could not set range start variable");
        sprintf(buffer, "%d", f.EpochDays());
        if (Tcl_SetVar(tcl, argv[1], buffer, 0) == NULL)
            TCL_Error(tcl, "could not set range finish variable");
        TCL_Return(tcl, "1");
    }
    else {
        TCL_Return(tcl, "0");
    }
}

static int item_date(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    int dateDays;
    if (Tcl_GetInt(tcl, argv[0], &dateDays) != TCL_OK) {
        TCL_Error(tcl, "invalid date");
    }
    Date date(dateDays);
    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_date(date);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_start(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    int dateDays;

    if (Tcl_GetInt(tcl, argv[0], &dateDays) != TCL_OK) {
        TCL_Error(tcl, "invalid date");
    }

    Date date(dateDays);
    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_start(date);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_finish(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    int dateDays;

    if (Tcl_GetInt(tcl, argv[0], &dateDays) != TCL_OK) {
        TCL_Error(tcl, "invalid date");
    }

    Date date(dateDays);
    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_finish(date);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_ondel(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;
    int dateDays;
    if (Tcl_GetInt(tcl, argv[0], &dateDays) != TCL_OK) {
        TCL_Error(tcl, "invalid date");
    }
    Date date(dateDays);
    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->delete_occurrence(date);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_dayr(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int interval;
    if ((Tcl_GetInt(tcl, argv[0], &interval) != TCL_OK) || (interval < 1)) {
        TCL_Error(tcl, "invalid interval");
    }

    int anchorDays;
    if (Tcl_GetInt(tcl, argv[1], &anchorDays) != TCL_OK) {
        TCL_Error(tcl, "invalid anchor date");
    }
    Date anchor(anchorDays);

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_day_based_repeat(interval, anchor);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_monthr(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int interval;
    if ((Tcl_GetInt(tcl, argv[0], &interval) != TCL_OK) || (interval < 1)) {
        TCL_Error(tcl, "invalid interval");
    }

    int anchorDays;
    if (Tcl_GetInt(tcl, argv[1], &anchorDays) != TCL_OK) {
        TCL_Error(tcl, "invalid anchor date");
    }
    Date anchor(anchorDays);

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_month_based_repeat(interval, anchor);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

// Helper routine for "set_monthly" interfaces.
// modifies     "n", "anchor", "interval".
// effects      Parses "n", "anchor" and "interval" from the command
//              arguments.  Returns TCL_OK iff successful.  If "anchor"
//              and "interval" are not specified in the command arguments,
//              then "anchor" is set to today and "interval" is set to "1".
static int parse_month_args(Tcl_Interp* tcl, int argc, const char* argv[],
                            int& n, Date& anchor, int& interval)
{
    if ((argc != 1) && (argc != 3)) TCL_Error(tcl,"wrong number of arguments");
    if ((Tcl_GetInt(tcl, argv[0], &n) != TCL_OK) || (n < 1))
        TCL_Error(tcl, "invalid count");

    // Get anchor and interval
    if (argc == 1) {
        anchor = Date::Today();
        interval = 1;
    }
    else {
        int anchorDays;
        if (Tcl_GetInt(tcl, argv[1], &anchorDays) != TCL_OK)
            TCL_Error(tcl, "invalid anchor date");
        anchor = Date(anchorDays);

        if ((Tcl_GetInt(tcl, argv[2], &interval) != TCL_OK) || (interval < 1))
            TCL_Error(tcl, "invalid interval");
    }

    return TCL_OK;
}

static int item_mday(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int n, interval;
    Date anchor;
    if (parse_month_args(tcl, argc, argv, n, anchor, interval) != TCL_OK)
        return TCL_ERROR;

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_monthly_by_days(n, interval, anchor, 0);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_mlday(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int n, interval;
    Date anchor;
    if (parse_month_args(tcl, argc, argv, n, anchor, interval) != TCL_OK)
        return TCL_ERROR;

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_monthly_by_days(n, interval, anchor, 1);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_mworkday(ClientData c, Tcl_Interp* tcl,
                         int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int n, interval;
    Date anchor;
    if (parse_month_args(tcl, argc, argv, n, anchor, interval) != TCL_OK)
        return TCL_ERROR;

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_monthly_by_workdays(n, interval, anchor, 0);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_mlworkday(ClientData c, Tcl_Interp* tcl,
                          int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int n, interval;
    Date anchor;
    if (parse_month_args(tcl, argc, argv, n, anchor, interval) != TCL_OK)
        return TCL_ERROR;

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_monthly_by_workdays(n, interval, anchor, 1);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_mweekday(ClientData c, Tcl_Interp* tcl,
                         int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int wday;
    if ((Tcl_GetInt(tcl,argv[0],&wday) != TCL_OK) || (wday < 1) || (wday > 7))
        TCL_Error(tcl, "invalid weekday");
    WeekDay w = WeekDay::First() + (wday - 1);

    int n, interval;
    Date anchor;
    if (parse_month_args(tcl, argc-1, argv+1, n, anchor, interval) != TCL_OK)
        return TCL_ERROR;

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_monthly_by_weeks(n, w, interval, anchor, 0);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_mlweekday(ClientData c, Tcl_Interp* tcl,
                          int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    int wday;
    if ((Tcl_GetInt(tcl,argv[0],&wday) != TCL_OK) || (wday < 1) || (wday > 7))
        TCL_Error(tcl, "invalid weekday");
    WeekDay w = WeekDay::First() + (wday - 1);

    int n, interval;
    Date anchor;
    if (parse_month_args(tcl, argc-1, argv+1, n, anchor, interval) != TCL_OK)
        return TCL_ERROR;

    if (! check_permission(tcl, item)) return TCL_ERROR;
    item->value()->set_monthly_by_weeks(n, w, interval, anchor, 1);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int item_wdays(ClientData c, Tcl_Interp* tcl, int argc, const char** argv) {
    Item_Tcl* item = (Item_Tcl*) c;

    /* Collect weekdays */
    int i;
    SmallIntSet set;
    set.Clear();
    for (i = 0; i < argc; i++) {
        int weekday;
        if ((Tcl_GetInt(tcl, argv[i], &weekday) != TCL_OK) ||
            (weekday < 1) ||
            (weekday > 7)) {
            TCL_Error(tcl, "invalid weekday");
        }
        set.Insert(weekday);
    }

    if (! check_permission(tcl, item)) return TCL_ERROR;

    /* Repeat every month */
    SmallIntSet months;
    months.Clear();
    for (i = 1; i <= 12; i++) {
        months.Insert(i);
    }

    item->value()->set_week_set(set, months);
    trigger_item(tcl, item);

    TCL_Return(tcl, "");
}

static int check_permission(Tcl_Interp* tcl, Item_Tcl* item) {
    CalFile* file = item->calendar();
    if (file == 0) return 1;

    if (file->GetCalendar()->ReadOnly()) {
        Tcl_SetResult(tcl, const_cast<char*>("item is in readonly calendar"),
                TCL_STATIC);
        return 0;
    }

    file->Modified();
    return 1;
}

static void trigger_item(Tcl_Interp* tcl, Item_Tcl* item, char const* t) {
    if (item->calendar() != 0)
        trigger(tcl, t, item->handle());
}
