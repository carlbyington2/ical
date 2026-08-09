/* Copyright (c) 1994 by Sanjay Ghemawat */
#include <stdio.h>
#include <string.h>

#include "collect.h"
#include "ical.h"
#include "cal_tcl.h"
#include "item_tcl.h"

#include "arrays.h"
#include "calfile.h"
#include "calendar.h"
#include "dateset.h"
#include "item.h"

implementArray(ItemList,Item_Tcl*)
implementArray(Occurrences,Occurrence)

void collect_all(Calendar_Tcl* cal, ItemList& list, bool only_visible) {
    Calendar* mainCalendar = cal->main->GetCalendar();

    for (int i = 0; i <= cal->includes->size(); i++) {
        Calendar* calendar = ((i >= cal->includes->size())
                              ? mainCalendar
                              : cal->includes->slot(i)->GetCalendar()
                              );
        if (only_visible && !calendar->Visible()) continue;
        collect_calendar(cal, calendar, list);
    }
}

void collect_calendar(Calendar_Tcl* cal, Calendar* calendar, ItemList& list) {
    Calendar* mainCalendar = cal->main->GetCalendar();

    for (int i = 0; i < calendar->Size(); i++) {
        Item_Tcl* item = Item_Tcl::find(calendar->Get(i));
        if (item == 0) continue;

        // Ignore hidden items
        if (mainCalendar->Hidden(item->value()->GetUid())) continue;

        list.append(item);
    }
}


void collect_occurrences(Calendar_Tcl* cal, ItemList const& items,
                         Occurrences& list,
                         Date s, Date f, int e)
{
    list.clear();
    for (int i = 0; i < items.size(); i++) {
        Item_Tcl* item = items[i];

        Date d = s - 1;
        Date limit = f;
        if (e) limit += item->value()->GetRemindStart();

        while (item->value()->next(d, d) && (d <= limit)) {
            Occurrence o;
            o.item = item;
            o.date = d;
            list.append(o);
        }
    }
}

static int occurs_before(Occurrence const& xo, Occurrence const& yo) {
    // Compare by date
    if (xo.date < yo.date) return 1;
    if (xo.date > yo.date) return 0;

    Item* x = xo.item->value();
    Item* y = yo.item->value();

    // Compare by appt time
    Appointment* xa = x->AsAppointment();
    Appointment* ya = y->AsAppointment();
    int xs = (xa == 0) ? -1 : xa->GetStart(true);
    int ys = (ya == 0) ? -1 : ya->GetStart(true);
    if (xs < ys) return 1;
    if (xs > ys) return 0;

    // Done items occur before todo items
    if (x->IsDone() && !y->IsDone()) return 1;
    if (y->IsDone() && !x->IsDone()) return 0;

    // Todo items occur before non-todo items
    if (x->IsTodo() && !y->IsTodo()) return 1;
    if (y->IsTodo() && !x->IsTodo()) return 0;

    return (strcmp(x->GetText(), y->GetText()) < 0);
}

void sort_occurrences(Occurrences& list) {
    int i = 0;
    while (i < list.size()) {
        /* Find min element in list[i..] */
        int minIndex = i;

        for (int j = i+1; j < list.size(); j++) {
            if (occurs_before(list[j], list[minIndex])) {
                minIndex = j;
            }
        }

        Occurrence temp;
        temp = list[i];
        list[i] = list[minIndex];
        list[minIndex] = temp;

        i++;
    }
}

void reverse(Occurrences& list) {
    // Scan from front and back of list while swapping elements
    int first = 0;
    int last = list.size() - 1;
    while (first < last) {
        Occurrence tmp = list[first];
        list[first] = list[last];
        list[last] = tmp;
        first++;
        last--;
    }
}

void trigger(Tcl_Interp* tcl, char const* ttype, char const* id) {
    charArray buffer;
    char const* cmd = "trigger fire ";

    buffer.concat(cmd, strlen(cmd));
    buffer.concat(ttype, strlen(ttype));

    if (id != 0) {
        buffer.append(' ');
        buffer.concat(id, strlen(id));
    }

    buffer.append('\0');

    if (Tcl_Eval(tcl, buffer.as_pointer()) == TCL_ERROR)
        fprintf(stderr, "ical: trigger error: %s\n", Tcl_GetStringResult(tcl));

    buffer.clear();
}
