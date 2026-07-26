/* Copyright (c) 1993 by Sanjay Ghemawat */

#ifndef _ITEMH
#define _ITEMH

#include "Date.h"

#include "dateset.h"
#include "smallintset.h"

class Lexer;
class charArray;
class intArray;
class OptionMap;

class Notice;
class Appointment;
struct Item_OldDates;

/*
 * Item
 *
 * Abstract components.
 *
 * Text: string			associated text
 * Dates: DateSet		occurrence dates
 * RemindStart: int		No. of days before item when reminder begins.
 * Owner: string		User id of person who last changed the item
 * Uid: string			Unique id for item
 * Hilite: string		Hilite mode
 * Todo: bool			Is Item a to-do item
 * Done: bool			Has item been marked "done"
 *
 * AsNotice: Notice*		Typecast to Notice if legal.  Null otherwise.
 * AsAppointment: Appointment*	Typecast to Appointment if legal ... 
 *
 */

class Item {
  public:
    Item();
    virtual ~Item();

    virtual int Read(Lexer*);
    virtual int Parse(Lexer*, char const* keyword, Item_OldDates&);
    virtual void Write(charArray*, int major, int minor) const;

    virtual Item* Clone() const = 0;

    const char* GetText() const;
    void SetText(const char*);

    // DateSet interface
    int contains(Date d) const;
    int repeats() const;
    int empty() const;
    DateSet::RepeatType repeat_type() const;
    void describe(charArray* buffer) const;

    int first(Date& result) const;
    int next(Date d, Date& result) const;
    int range(Date&, Date&) const;

    void set_empty();
    void set_date(Date d);
    void set_day_based_repeat(int interval, Date anchor);

    void set_month_based_repeat(int interval, Date anchor);
    void set_monthly_by_days(int c, int i, Date anchor, int back);
    void set_monthly_by_workdays(int c, int i, Date anchor, int back);
    void set_monthly_by_weeks(int c, WeekDay, int i, Date anchor, int back);

    void set_month_set(SmallIntSet days, SmallIntSet months);
    void set_week_set(SmallIntSet days, SmallIntSet months);

    void set_start(Date start);
    void set_finish(Date finish);

    void delete_occurrence(Date d);

    int GetRemindStart() const;
    void SetRemindStart(int);

    const char* GetOwner() const;
    // effects - Return the owner.  The returned string is guaranteed
    //		 to remain valid until the item is modified or deleted.

    void SetOwner(char const* o);
    // modifies - "this"
    // effects	- Make "o" the owner of the item.

    void MakeOwner();
    // modifies - this
    // effects	- Make the current user the owner of this item.

    int IsMine() const;
    // effects	- Returns true iff this item is owned by the current user.

    char const* GetUid() const;
    // effects - Return the uid.  The returned string is guaranteed
    //		 to remain valid until the item is deleted.

    //void SetUid(char const*);
    // modifies - this
    // effects  - Sets uid to specified value.

    int IsUidPersistent() const;
    // effects - Return true iff uid is also stored persistently

    virtual Notice* AsNotice();
    virtual Appointment* AsAppointment();

    /* Mark deleted */
    int Deleted();
    void MarkDeleted();

    /* Hilite mode */
    char const* Hilite() const;
    void Hilite(char const*);

    /* Todo? */
    int IsTodo() const;
    void SetTodo(int t);

    /* Done? */
    int IsDone() const;
    void SetDone(int d);

    /* Options */
    char const* GetOption(char const* key) const;
    // effects	- Return value associated with option named "key".
    //		  Returns 0 if option is not found.

    void SetOption(char const* key, char const* value);
    // modifies	- this
    // effects	- add "<key, value>" to option list.

    void RemoveOption(char const* key);
    // modifies	- this
    // effects	- Remove any option associated with "key"

    /* Comparison */
    int similar(Item const* x) const;
    // effects	Returns true iff this has same contents as x.

    static const int defaultRemindStart;
  protected:
    char*	text;
    char*       owner;
    char*	uid;
    int		uid_persistent;
    int         remindStart;	/* Start reminding early */
    int         deleted;
    DateSet*    date;
    char*	hilite;
    int		todo;
    int		done;
    OptionMap*	options;

    void CopyTo(Item*) const;
};

/*
 * Notice
 *
 * An Item with just a length.
 * The length is just an indication of how big a window the notice must
 * be displayed in.  The length field can be interpreted just as the length
 * field for Appointments, and if notice->length == appt->length, then
 * the notice will be displayed the same size as the appt.
 */
class Notice : public Item {
  public:
    Notice();
    ~Notice();

    virtual int Parse(Lexer*, char const* keyword, struct Item_OldDates&);
    virtual void Write(charArray*, int major, int minor) const;

    virtual Item* Clone() const;

    int GetLength() const;
    void SetLength(int);

    virtual Notice* AsNotice();
  protected:
    int length;
};

/*
 * Appointment
 *
 * An Item with a start time and a length.
 * Both start and length are in minutes.
 * Start is measured from midnight.
 */
class Appointment : public Item {
  public:
    Appointment();
    ~Appointment();

    virtual int Parse(Lexer*, char const* keyword, struct Item_OldDates&);
    virtual void Write(charArray*, int major, int minor) const;

    virtual Item* Clone() const;

    int GetStart() const;
    int GetLength() const;
    int GetFinish() const;
    void SetStart(int);
    void SetLength(int);

    intArray* GetAlarms() const;
    // effects - Return array of alarm times.  If no alarm times
    //		 have been set for this item, return NULL.
    //		 The returned array is guaranteed to remain
    //		 valid until the item is modified or deleted.

    void SetAlarms(intArray* list);
    // modifies	- this
    // effects	- Sets alarms for this item from the contents of list.

    virtual Appointment* AsAppointment();
  protected:
    int start;
    int length;
    intArray* alarms;		/* Alarm times */
};

// Inlined routines

inline char const* Item::GetUid() const {
    return uid;
}

#endif /* _ITEMH */
