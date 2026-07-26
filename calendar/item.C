/* Copyright (c) 1993 by Sanjay Ghemawat */

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "basic.h"
#include "Array.h"

#include "Year.h"
#include "Month.h"
#include "WeekDay.h"
#include "Date.h"

#include "arrays.h"
#include "item.h"
#include "lexer.h"
#include "misc.h"
#include "options.h"
#include "uid.h"
#include "version.h"

#define HTABLE_IMPLEMENT
#include "itemmap.h"

/*
 * Info needed for old date set format.
 */
struct Item_OldDates {
    int		inited;
    int		isWeekly;
    SmallIntSet	days;
    SmallIntSet months;
    int		everyYear;
    int		firstYear;
    int		lastYear;

    DateList	deleteList;
};

const int Item::defaultRemindStart = 1;

static const char opener = '[';
static const char closer = ']';

Item::Item() {
    text = copy_string("");
    owner = copy_string("");

    uid = (char*) uid_new();
    uid_persistent = 0;

    deleted = 0;
    remindStart = defaultRemindStart;

    date = new DateSet;

    hilite = copy_string("always");
    todo = 0;
    done = 0;
    options = 0;
}

Item::~Item() {
    delete [] text;
    delete [] owner;
    delete [] uid;
    delete [] hilite;
    delete date;

    if (options != 0) delete options;
}

int Item::Read(Lexer* lex) {
    Item_OldDates old;
    old.inited = 0;
    old.isWeekly = 0;
    old.days.Clear();
    old.months.Clear();
    old.everyYear = 1;

    while (1) {
	char c;
	char const* keyword;

	if (! lex->SkipWS() ||
	    ! lex->Peek(c)) {
	    lex->SetError("incomplete item");
	    return 0;
	}

	if (c == closer) {
	    /*
	     * Item is over.  Convert old date format into new format
	     * if necessary.
	     */

	    if (old.inited) {
		if (old.isWeekly) {
		    date->set_week_set(old.days, old.months);
		}
		else {
		    date->set_month_set(old.days, old.months);
		}
		if (! old.everyYear) {
		    date->set_start(Date(1,Month::January(),old.firstYear));
		    date->set_finish(Date(31,Month::December(),old.lastYear));
		}

		for (int i = 0; i < old.deleteList.size(); i++) {
		    date->delete_occurrence(old.deleteList[i]);
		}
	    }

	    return 1;
	}

	if (! lex->GetId(keyword) ||
	    ! lex->SkipWS() ||
	    ! lex->Skip(opener)) {
	    lex->SetError("error reading item property name");
	    return 0;
	}

	if (! Parse(lex, keyword, old) ||
	    ! lex->SkipWS() ||
	    ! lex->Skip(closer)) {
	    lex->SetError("error reading item property");
	    return 0;
	}
    }
}

int Item::Parse(Lexer* lex, char const* keyword, Item_OldDates& old) {
    if (strcmp(keyword, "Remind") == 0) {
	if (! lex->SkipWS() ||
	    ! lex->GetNumber(remindStart)) {
	    lex->SetError("error reading remind level");
	    return 0;
	}
	return 1;
    }

    if (strcmp(keyword, "Owner") == 0) {
	char const* x;
	if (!lex->GetString(x)) {
	    lex->SetError("error reading owner information");
	    return 0;
	}
	delete [] owner;
	owner = copy_string(x);
	return 1;
    }

    if (strcmp(keyword, "Uid") == 0) {
	char const* x;
	if (!lex->SkipWS() || !lex->GetUntil(closer, x)) {
	    lex->SetError("error reading unique id");
	    return 0;
	}
	delete [] uid;
	uid = copy_string(x);
	uid_persistent = 1;
	return 1;
    }

    if (strcmp(keyword, "Contents") == 0) {
	char const* x;
	if (! lex->GetString(x)) {
	    lex->SetError("error reading item text");
	    return 0;
	}
	SetText(x);
	return 1;
    }

    if (strcmp(keyword, "Text") == 0) {
	/* Read text */
	int len;

	// Read length
	if (! lex->SkipWS() ||
	    ! lex->GetNumber(len) ||
	    ! (len >= 0) ||
	    ! lex->SkipWS() ||
	    ! lex->Skip(opener)) {
	    lex->SetError("error reading item text");
	    return 0;
	}

	// Allocate enough space for text
	delete [] text;
	text = new char[len+1];
	strcpy(text, "");

	if (! lex->GetText(text, len) ||
	    ! lex->Skip(closer)) {
	    delete [] text;
	    text = copy_string("");
	    lex->SetError("error reading item text");
	    return 0;
	}

	text[len] = '\0';
	return 1;
    }

    if (strcmp(keyword, "Dates") == 0) {
	int start = lex->Index();
	if (! date->read(lex)) {
	    // Could not understand date format.
	    // Stash it away as an option.
	    lex->Reset(start);
	    char const* val;
	    if (! lex->GetString(val)) {
		lex->SetError("error reading date information");
		return 0;
	    }
	    if (options == 0) options = new OptionMap;
	    options->store("Dates", val);
	}
	return 1;
    }

    if (strcmp(keyword, "Wdays") == 0) {
	old.inited = 1;
	old.isWeekly = 1;
	if (! old.days.Read(lex)) {
	    lex->SetError("error reading weekdays");
	    return 0;
	}
	return 1;
    }

    if (strcmp(keyword, "Mdays") == 0) {
	old.inited = 1;
	old.isWeekly = 0;
	if (! old.days.Read(lex)) {
	    lex->SetError("error reading monthdays");
	    return 0;
	}
	return 1;
    }

    if (strcmp(keyword, "Months") == 0) {
	old.inited = 1;
	if (! old.months.Read(lex)) {
	    lex->SetError("error reading set of months");
	    return 0;
	}
	return 1;
    }

    if (strcmp(keyword, "EveryYear") == 0) {
	old.inited = 1;
	old.everyYear = 1;
	return 1;
    }

    if (strcmp(keyword, "Years") == 0) {
	old.inited = 1;
	old.everyYear = 0;

	if (! lex->SkipWS() ||
	    ! lex->GetNumber(old.firstYear) ||
	    ! lex->SkipWS() ||
	    ! lex->GetNumber(old.lastYear)) {
	    lex->SetError("error reading range of years");
	    return 0;
	}
	return 1;
    }

    if (strcmp(keyword, "Deleted") == 0) {
	int day, month, year;

	if (! lex->SkipWS() ||
	    ! lex->GetNumber(day) ||
	    ! lex->SkipWS() ||
	    ! lex->GetNumber(month) ||
	    ! lex->SkipWS() ||
	    ! lex->GetNumber(year)) {
	    lex->SetError("error reading deletion date");
	    return 0;
	}

	old.inited = 1;
	old.deleteList.append(Date(day, Month::First()+(month-1), year));
	return 1;
    }

    if (strcmp(keyword, "Hilite") == 0) {
	char const* x;
	if (!lex->GetString(x)) {
	    lex->SetError("error reading item hilite");
	    return 0;
	}

	delete [] hilite;
	hilite = copy_string(x);
	return 1;
    }

    if (strcmp(keyword, "Todo") == 0) {
	todo = 1;
	return 1;
    }

    if (strcmp(keyword, "Done") == 0) {
	done = 1;
	return 1;
    }

    char* key = copy_string(keyword);
    char const* val;
    if (! lex->GetString(val)) {
	lex->SetError("error reading item property");
	delete [] key;
	return 0;
    }

    if (options == 0) options = new OptionMap;
    options->store(key, val);
    delete [] key;
    return 1;
}

void Item::Write(charArray* out, int major, int minor) const {
    format(out, "Uid [%s]\n", uid);
    ((Item*) this)->uid_persistent = 1;

    if (strlen(owner) != 0) {
	append_string(out, "Owner [");
	Lexer::PutString(out, owner);
	append_string(out, "]\n");
    }

    if (major == 1) {
	format(out, "Text [%d [", strlen(text));
	append_string(out, text);
	append_string(out, "]]\n");
    } else {
	append_string(out, "Contents [");
	Lexer::PutString(out, text);
	append_string(out, "]\n");
    }

    format(out, "Remind [%d]\n", remindStart);

    append_string(out, "Hilite [");
    Lexer::PutString(out, hilite);
    append_string(out, "]\n");

    if (todo) {append_string(out, "Todo []\n");}
    if (done) {append_string(out, "Done []\n");}

    append_string(out, "Dates [");
    date->write(out);
    append_string(out, "]\n");

    if (options != 0) {
	options->write(out);
    }
}

void Item::CopyTo(Item* item) const {
    // The code below cannot correctly handle aliasing.
    if (item == this) return;

    item->SetText(text);
    item->Hilite(hilite);
    item->SetOwner(owner);

    item->todo = todo;
    item->done = done;
    *item->date = *date;
    item->remindStart = remindStart;

    // Do NOT copy uid.  That would defeat the whole purpose of uids

    // Clear any options in the destination
    if (item->options != 0) {
	delete item->options;
	item->options = 0;
    }

    // Copy the option map
    if (options != 0) {
	item->options = new OptionMap;
	for (OptionMap_Bindings o = options->bindings(); o.ok(); o.next()) {
	    item->options->store(o.key(), o.val());
	}
    }
}

const char* Item::GetText() const {
    return text;
}

void Item::SetText(const char* t) {
    char* copy = copy_string(t);
    delete [] text;
    text = copy;
}

int Item::GetRemindStart() const {
    return remindStart;
}

void Item::SetRemindStart(int r) {
    remindStart = r;
}

char const* Item::GetOwner() const {
    return owner;
}

void Item::SetOwner(char const* o) {
    char* copy = copy_string(o);
    delete [] owner;
    owner = copy;
}

void Item::MakeOwner() {
    char const* id = my_name();
    if (id != 0) {
	delete [] owner;
	owner = copy_string(id);
    }
}

int Item::IsMine() const {
    char const* id = my_name();
    if ((id == 0) || (owner == 0)) {
	// Unknown owner
	return 0;
    }

    return (strcmp(id, owner) == 0);
}

#if 0
void Item::SetUid(char const* u) {
    // Make a copy before deleting the old uid because the old uid and "u"
    // may be aliases of the same string.
    char* new_uid = copy_string(u);
    delete [] uid;
    uid = new_uid;
}
#endif

int Item::IsUidPersistent() const {
    return uid_persistent;
}

Notice* Item::AsNotice() {
    return 0;
}

Appointment* Item::AsAppointment() {
    return 0;
}

int Item::Deleted() {
    return deleted;
}

void Item::MarkDeleted() {
    deleted = 1;
}

char const* Item::Hilite() const {
    return hilite;
}

void Item::Hilite(char const* h) {
    char* copy = copy_string(h);
    delete [] hilite;
    hilite = copy;
}

int Item::IsTodo() const {
    return todo;
}

void Item::SetTodo(int t) {
    todo = t;
    if (!todo) done = 0;
}

int Item::IsDone() const {
    return done;
}

void Item::SetDone(int d) {
    done = d;
    if (!todo) done = 0;
}

char const* Item::GetOption(char const* key) const {
    char const* val;
    if (options == 0) return 0;
    if (options->fetch(key, val)) return val;
    return 0;
}

void Item::SetOption(char const* key, char const* val) {
    if (options == 0) options = new OptionMap;
    options->store(key, val);
}

void Item::RemoveOption(char const* key) {
    if (options != 0) options->remove(key);
}

int Item::similar(Item const* x) const {
    // Fast check
    if (this == x) return 1;

    // XXX Just compare unparsing: only works if it is deterministic
    charArray aval, bval;
    this->Write(&aval, VersionMajor, VersionMinor);
    x->Write(&bval, VersionMajor, VersionMinor);

    if (aval.size() != bval.size()) return 0;
    return strncmp(aval.as_pointer(), bval.as_pointer(), aval.size());
}

Notice::Notice() {
    length = 30;
}

Notice::~Notice() { }

int Notice::Parse(Lexer* lex, char const* keyword, Item_OldDates& old) {
    if (strcmp(keyword, "Length") == 0) {
	if (! lex->SkipWS() ||
	    ! lex->GetNumber(length)) {
	    lex->SetError("error reading notice display length");
	    return 0;
	}
	return 1;
    }

    return Item::Parse(lex, keyword, old);
}

void Notice::Write(charArray* out, int major, int minor) const {
    format(out, "Length [%d]\n", length);
    Item::Write(out, major, minor);
}

Item* Notice::Clone() const {
    Notice* copy = new Notice;

    Item::CopyTo(copy);
    copy->SetLength(length);
    return copy;
}

int Notice::GetLength() const {
    return length;
}

void Notice::SetLength(int l) {
    length = l;
}

Notice* Notice::AsNotice() {
    return this;
}

Appointment::Appointment() {
    start = 30;
    length = 30;
    alarms = 0;
}

Appointment::~Appointment() {
    if (alarms != 0) {
	delete alarms;
    }
}

int Appointment::Parse(Lexer* lex, char const* keyword, Item_OldDates& old) {
    if (strcmp(keyword, "Start") == 0) {
	if (! lex->SkipWS() ||
	    ! lex->GetNumber(start)) {
	    lex->SetError("error reading appointment start time");
	    return 0;
	}
	return 1;
    }

    if (strcmp(keyword, "Length") == 0) {
	if (! lex->SkipWS() ||
	    ! lex->GetNumber(length)) {
	    lex->SetError("error reading appointment length");
	    return 0;
	}
	return 1;
    }

    if (strcmp(keyword, "Alarms") == 0) {
	if (alarms == 0) alarms = new intArray;
	alarms->clear();

	while (1) {
	    char c;

	    lex->SkipWS();
	    if (! lex->Peek(c)) {
		lex->SetError("error reading alarm list");
		return 0;
	    }

	    if (!isdigit(c)) break;

	    int num;
	    if (! lex->GetNumber(num)) return 0;
	    alarms->append(num);
	}

	return 1;
    }

    return Item::Parse(lex, keyword, old);
}

void Appointment::Write(charArray* out, int major, int minor) const {
    format(out, "Start [%d]\n", start);
    format(out, "Length [%d]\n", length);
    if (alarms != 0) {
	append_string(out, "Alarms [");
	for (int i = 0; i < alarms->size(); i++) {
	    int x = alarms->slot(i);
	    format(out, " %d", x);
	}
	append_string(out, "]\n");
    }

    Item::Write(out, major, minor);
}

Item* Appointment::Clone() const {
    Appointment* copy = new Appointment;

    Item::CopyTo(copy);
    copy->SetStart(start);
    copy->SetLength(length);

    if (copy->alarms != 0) {
	delete copy->alarms;
	copy->alarms = 0;
    }
    if (alarms != 0) {
	copy->alarms = new intArray(*alarms);
    }

    return copy;
}

int Appointment::GetStart() const {
    return start;
}

void Appointment::SetStart(int s) {
    start = s;
}

int Appointment::GetLength() const {
    return length;
}

void Appointment::SetLength(int l) {
    length = l;
}

int Appointment::GetFinish() const {
    return start + length;
}

intArray* Appointment::GetAlarms() const {
    return alarms;
}

void Appointment::SetAlarms(intArray* list) {
    if (alarms == 0) alarms = new intArray;
    alarms->clear();
    *alarms = *list;
}

Appointment* Appointment::AsAppointment() {
    return this;
}

// DateSet wrappers

int Item::contains(Date d) const {
    if (!todo || done) return date->contains(d);

    // Special handling for todo items
    Date today = Date::Today();
    if (d < today) return 0;
    if (d > today) return date->contains(d);

    // d == today
    Date f;
    return (date->first(f) && (f <= today));
}

int Item::repeats() const {
    return date->repeats();
}

int Item::empty() const {
    return date->empty();
}

DateSet::RepeatType Item::repeat_type() const {
    return date->type();
}

void Item::describe(charArray* buffer) const {
    date->describe(buffer);
}

int Item::first(Date& result) const {
    if (!date->first(result)) return 0;
    if (!todo || done) return 1;

    // Special handling for todo items
    Date today = Date::Today();
    if (result < today) result = today;
    return 1;
}

int Item::range(Date& s, Date& f) const {
    if (!date->get_range(s, f)) return 0;
    if (!todo || done) return 1;

    // Special handling for todo items
    Date today = Date::Today();
    if (s < today) s = today;
    return 1;
}

int Item::next(Date d, Date& result) const {
    if (!todo || done) return date->next(d, result);

    // Special handling for todo items
    Date today = Date::Today();
    if (d >= today) return date->next(d, result);

    // Starting search before "today" -- just find first item
    if (!date->first(result)) return 0;
    if (result < today) result = today;
    return 1;
}

void Item::set_empty() {
    date->set_empty();
}

void Item::set_date(Date d) {
    date->set_date(d);
}

void Item::set_day_based_repeat(int interval, Date anchor) {
    date->set_day_based_repeat(interval, anchor);
}

void Item::set_month_based_repeat(int interval, Date anchor) {
    date->set_month_based_repeat(interval, anchor);
}

void Item::set_monthly_by_days(int c, int i, Date a, int b) {
    date->set_monthly_by_days(c, i, a, b);
}

void Item::set_monthly_by_workdays(int c, int i, Date a, int b) {
    date->set_monthly_by_workdays(c, i, a, b);
}

void Item::set_monthly_by_weeks(int c, WeekDay w, int i, Date a, int b) {
    date->set_monthly_by_weeks(c, w, i, a, b);
}

void Item::set_week_set(SmallIntSet days, SmallIntSet months) {
    date->set_week_set(days, months);
}

void Item::set_month_set(SmallIntSet days, SmallIntSet months) {
    date->set_month_set(days, months);
}

void Item::set_start(Date start) {
    date->set_start(start);
}

void Item::set_finish(Date finish) {
    date->set_finish(finish);
}

void Item::delete_occurrence(Date d) {
    if (!todo || done) {
	date->delete_occurrence(d);
	return;
    }

    // Special handling for todo items
    Date today = Date::Today();
    if (d < today) return;
    if (d > today) {
	date->delete_occurrence(d);
	return;
    }

    // Deleting "today" -- do it by setting range
    Date f;
    if (date->first(f) && (f <= today)) date->set_start(today+1);
}
