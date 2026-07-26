/* Copyright (c) 1993 by Sanjay Ghemawat */
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "arrays.h"
#include "Month.h"
#include "WeekDay.h"
#include "Year.h"

#include "dateset.h"
#include "lexer.h"
#include "misc.h"

// Helper routines
static void advance_months(Month&, int&, int);

static void append_date(charArray* buffer, Date date) {
    int day, year;
    WeekDay wday;
    Month month;
    date.BreakDown(day, wday, month, year);

    format(buffer, "%s %s %d, %d", wday.Name(), month.Name(), day, year);
}

static void append_int(charArray* buffer, int i) {
    // Generate appropriate suffix based on value of "i"
    char const* suff = "th";

    if (((i/10) % 10) != 1) {
        // Second last digit is not "1".  Special case on the last digit.
        switch (i % 10) {
          case 1: suff = "st"; break;
          case 2: suff = "nd"; break;
          case 3: suff = "rd"; break;
        }
    }

    format(buffer, "%d%s", i, suff);
}

class DateSetRep {
  public:
    DateSetRep();
    virtual ~DateSetRep();

    virtual DateSetRep* copy() const = 0;
    virtual DateSet::RepeatType type() const = 0;
    virtual void describe(charArray*) const = 0;
    virtual void describe_terse(charArray*) const = 0;
    virtual int contains(Date) const = 0;
    virtual int search(Date, Date&) const = 0;

    virtual int  read(Lexer*) = 0;
    virtual void write(charArray*) const = 0;


    virtual int search(Date anchor, Date& result,
                       Date start, Date finish, DateList const& deleted) const;
    /*
     * modifies result
     * effects  Find smallest d such that d occurs in *this and d >= anchor
     *          and d is in [start..finish] and d is not in deleted.
     *          Set result to d and return 1 if successful.
     *          If no d could be found, return 0 without modifying result.
     */

    /* Default normalization does nothing */
    virtual DateSetRep* normalize(Date& start,
                                  Date& finish,
                                  DateList& deleted) const;
};

/*
 * DateSet(rep) = {}
 */
class EmptyDateSetRep : public DateSetRep {
  public:
    EmptyDateSetRep();
    virtual ~EmptyDateSetRep();

    virtual DateSetRep* copy() const;
    virtual DateSet::RepeatType type() const;
    virtual void describe(charArray*) const;
    virtual void describe_terse(charArray*) const;
    virtual int contains(Date) const;
    virtual int search(Date, Date&) const;

    virtual int read(Lexer*);
    virtual void write(charArray*) const;
};

/*
 * DateSet(rep) = if (deleted) then {} else { date }
 */
class SingleDateSetRep : public DateSetRep {
  public:
    SingleDateSetRep(Date);
    virtual ~SingleDateSetRep();

    virtual DateSetRep* copy() const;
    virtual DateSet::RepeatType type() const;
    virtual void describe(charArray*) const;
    virtual void describe_terse(charArray*) const;
    virtual int contains(Date) const;
    virtual int search(Date, Date&) const;

    virtual int read(Lexer*);
    virtual void write(charArray*) const;
  private:
    Date date;          /* Actual date */
    int  deleted;       /* Has date been deleted? */
};

/*
 * DateSet(rep) = { d | d = (anchor + k*interval) where k is an integer }
 */
class DayBasedDateSetRep : public DateSetRep {
  public:
    DayBasedDateSetRep(int, Date);
    virtual ~DayBasedDateSetRep();

    virtual DateSetRep* copy() const;
    virtual DateSet::RepeatType type() const;
    virtual void describe(charArray*) const;
    virtual void describe_terse(charArray*) const;
    virtual int contains(Date) const;
    virtual int search(Date, Date&) const;

    virtual int read(Lexer*);
    virtual void write(charArray*) const;
  private:
    int interval;
    Date anchor;
};

/*
 * For "k"th day|workday|weekday of each month (optionally counting
 * from end of month)
 */
enum MType {
    ByDay,
    ByWorkDay,
    ByWeek
};

// Complex month repetition class
class MonthBasedDateSetRep : public DateSetRep {
  public:
    // More complex repetition
    MonthBasedDateSetRep(int c, int i, Date a, MType t, WeekDay w, int b);

    virtual ~MonthBasedDateSetRep();

    virtual DateSetRep* copy() const;
    virtual DateSet::RepeatType type() const;
    virtual void describe(charArray*) const;
    virtual void describe_terse(charArray*) const;
    virtual int contains(Date) const;
    virtual int search(Date, Date&) const;

    virtual int read(Lexer*);
    virtual void write(charArray*) const;
  protected:
    int find_in_month(Month m, int y) const;

    int         count;                  // Count of days|workdays|weeks
    int         interval;               // Month interval
    Date        anchor;                 // Anchor date
    Month       anchorMonth;            // Anchor month
    int         anchorYear;             // Anchor year
    MType       mtype;                  // Repetition type
    WeekDay     weekday;                // Week day for repeat by week
    int         backward;               // Search from end of month?
};

// Simple month repetition class
class MonthDateSetRep : public MonthBasedDateSetRep {
  public:
    MonthDateSetRep(int i, Date a);

    // Most routines are inherited from MonthBasedDatSetRep;

    // Special routines so that we can leave format unchanged
    // from Ical charArray format 1.6.
    virtual DateSetRep* copy() const;

    virtual int read(Lexer*);
    virtual void write(charArray*) const;
};

/*
 * DateSet(rep) = { d | wday(d) in days and month(d) in months
 *                      and in_range(d) and !is_deleted(d). }
 */
class WeekSetDateSetRep : public DateSetRep {
  public:
    WeekSetDateSetRep(SmallIntSet, SmallIntSet);
    virtual ~WeekSetDateSetRep();

    virtual DateSetRep* copy() const;
    virtual DateSet::RepeatType type() const;
    virtual void describe(charArray*) const;
    virtual void describe_terse(charArray*) const;
    virtual int contains(Date) const;
    virtual int search(Date, Date&) const;

    virtual int read(Lexer*);
    virtual void write(charArray*) const;

    virtual DateSetRep* normalize(Date&, Date&, DateList&) const;
  private:
    SmallIntSet days;
    SmallIntSet months;
};

/*
 * DateSet(rep) = { d | mday(d) in days and month(d) in months
 *                      and in_range(d) and !contains_date(deleted, d). }
 */
class MonthSetDateSetRep : public DateSetRep {
  public:
    MonthSetDateSetRep(SmallIntSet, SmallIntSet);
    virtual ~MonthSetDateSetRep();

    virtual DateSetRep* copy() const;
    virtual DateSet::RepeatType type() const;
    virtual void describe(charArray*) const;
    virtual void describe_terse(charArray*) const;
    virtual int contains(Date) const;
    virtual int search(Date, Date&) const;

    virtual int read(Lexer*);
    virtual void write(charArray*) const;

    virtual DateSetRep* normalize(Date&, Date&, DateList&) const;
  private:
    SmallIntSet days;
    SmallIntSet months;
};

static int contains_date(DateList const& list, Date d) {
    for (int i = 0; i < list.size(); i++) {
        if (list[i] == d) {
            return 1;
        }
    }
    return 0;
}

DateSet::DateSet() : normalized(1) {
    rep = new EmptyDateSetRep;
    start = Date::First();
    finish = Date::Last();
    deleted.clear();
}

DateSet::DateSet(DateSet const& d) {
    d.normalize();
    rep = d.rep->copy();
    start = Date::First();
    finish = Date::Last();
    deleted = d.deleted;
}

DateSet::~DateSet() {
  delete rep;
}

void DateSet::operator = (DateSet const& d) {
    d.normalize();
    normalized = 1;

    DateSetRep* oldrep = rep;
    rep = d.rep->copy();
    delete oldrep;

    start = d.start;
    finish = d.finish;
    deleted = d.deleted;
}

int DateSet::contains(Date d) const {
    normalize();
    return (rep->contains(d) &&
            (d >= start) &&
            (d <= finish) &&
            !contains_date(deleted, d));
}

int DateSet::repeats() const {
    normalize();

    return ((rep->type() != DateSet::None) && !empty());
}

int DateSet::empty() const {
    normalize();

    Date d;
    return (!first(d));
}

int DateSet::get_range(Date& s, Date& f) const {
    normalize();

    // Fail if no occurrences
    Date d;
    if (!first(d)) return 0;

    // Singleton range for singleton item
    if (!repeats()) {
        s = d;
        f = d;
        return 1;
    }

    // Otherwise use the clipping range
    s = start;
    f = finish;
    return 1;
}

int DateSet::first(Date& d) const {
    return (rep->search(start, d, start, finish, deleted));
}

int DateSet::next(Date d, Date& result) const {
    if (d == Date::Last()) {
        return 0;
    }
    else {
        return (rep->search(d+1, result, start, finish, deleted));
    }
}

int DateSet::occurs_before(DateSet const& d1, DateSet const& d2) {
    Date x1, x2;

    if (! d1.first(x1)) {
        /* No way d1 occurs before d2 */
        return 0;
    }
    if (! d2.first(x2)) {
        /* D1 has occurrence, D2 does not */
        return 1;
    }

    return (x1 < x2);
}

int DateSet::occurs_compare(DateSet const& d1, DateSet const& d2) {
    Date x1, x2;

    if (! d1.first(x1)) {
        if (d2.empty()) {
            /* Both d1 and d2 are empty */
            return 0;
        }
        /* d1 is empty, d2 is not */
        return 1;
    }

    if (! d2.first(x2)) {
        /* d1 occurs, but d2 does not */
        return -1;
    }

    return ((x1 < x2) ? -1 : ((x1 > x2) ? 1 : 0));
}

DateSet::RepeatType DateSet::type() const {
    normalize();
    return (rep->type());
}

void DateSet::describe(charArray* buffer) const {
    normalize();
    rep->describe(buffer);
}

void DateSet::describe_terse(charArray* buffer) const {
    normalize();
    format(buffer, "%d %d {", start.EpochDays(), finish.EpochDays());
    for (int i = 0; i < deleted.size(); i++)
        format(buffer, " %d", deleted[i].EpochDays());
    append_string(buffer, " } ");
    rep->describe_terse(buffer);
}

void DateSet::set_empty() {
    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;
    rep = new EmptyDateSetRep;
    normalized = 0;
}

void DateSet::set_date(Date d) {
    start = d;
    finish = d;
    deleted.clear();

    delete rep;
    rep = new SingleDateSetRep(d);
    normalized = 0;
}

void DateSet::set_day_based_repeat(int i, Date a) {
    assert(i > 0);

    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;
    rep = new DayBasedDateSetRep(i, a);
    normalized = 0;
}

void DateSet::set_month_based_repeat(int i, Date a) {
    assert (i > 0);

    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;
    rep = new MonthDateSetRep(i, a);
    normalized = 0;
}

void DateSet::set_week_set(SmallIntSet days, SmallIntSet months) {
    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;
    rep = new WeekSetDateSetRep(days, months);
    normalized = 0;
}

void DateSet::set_month_set(SmallIntSet days, SmallIntSet months) {
    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;
    rep = new MonthSetDateSetRep(days, months);
    normalized = 0;
}

void DateSet::set_monthly_by_days(int count, int i, Date a, int back) {
    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;

    // Check for simple case
    if (!back && (count > 0) && (count <= a.GetMonth().Size(a.GetYear()))) {
        a = Date(count, a.GetMonth(), a.GetYear());
        rep = new MonthDateSetRep(i, a);
    }
    else {
        rep = new MonthBasedDateSetRep(count, i, a, ByDay, WeekDay::First(),
                                       back);
    }
    normalized = 0;
}

void DateSet::set_monthly_by_workdays(int count, int i, Date a, int back) {
    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;
    rep = new MonthBasedDateSetRep(count, i, a, ByWorkDay,
                                 WeekDay::First(), back);
    normalized = 0;
}

void DateSet::set_monthly_by_weeks(int count, WeekDay w, int i, Date a,
                                   int back) {
    start = Date::First();
    finish = Date::Last();
    deleted.clear();

    delete rep;
    rep = new MonthBasedDateSetRep(count, i, a, ByWeek, w, back);
    normalized = 0;
}

void DateSet::normalize() const {
    int i;
    if (! normalized) {
        /* Cast away const-ness */
        DateSet* self = (DateSet*) this;

        self->normalized = 1;

        /* Common normalizations */

        Date f, s;
        if (! first(f)) {
            delete rep;
            self->rep = new EmptyDateSetRep;
            self->start = Date::First();
            self->finish = Date::Last();
            self->deleted.clear();
        }
        else {
            if (! next(f, s)) {
                delete rep;
                self->rep = new SingleDateSetRep(f);
                self->start = Date::First();
                self->finish = Date::Last();
                self->deleted.clear();
            }
        }

        /* Type-specific normalizations */

        DateSetRep* newRep;
        while ((newRep = self->rep->normalize(self->start,
                                              self->finish,
                                              self->deleted)) != 0) {
            delete self->rep;
            self->rep = newRep;
        }

        /* Normalize start date */
        if (start != Date::First()) {
            Date d;
            if (!rep->search(Date::First(), d) || (d >= start)) {
                /* Nothing exists before start anyway */
                self->start = Date::First();
            }
        }

        /* Normalize finish date */
        if (finish != Date::Last()) {
            Date d;
            if (!rep->search(finish+1, d)) {
                /* Nothing exists after finish anyway */
                self->finish = Date::Last();
            }
        }

        // Trim delete list by setting "start" date
        int dcount = 0;
        for (i = 0; i < deleted.size(); i++)
            if (deleted[i] < f) dcount++;

        if (dcount > 0) {
            // At least one of the deleted dates occurs before
            // the current first date.  We can clip off all deleted
            // dates before "f" by setting "start" to "f".
            self->start = f;
            i = 0;
            while (i < deleted.size()) {
                if (deleted[i] < f) {
                    deleted[i] = deleted.high();
                    self->deleted.remove();
                    continue;
                }
                i++;
            }
        }
    }
}

/*
 * String format is
 *
 * <format> -> <type> [Start <date>] [Finish <date>] [Deleted <date>] End
 * <type>   -> Empty
 *          |  Single <date>
 *          |  Days <date> <int>
 *          |  Months <date> <int>
 *          |  WeekDays <set> Months <set>
 *          |  MonthDays <set> Months <set>
 *
 * <date>   -> <day>/<month>/<year>
 * <day>    -> <int>
 * <month>  -> <int>
 * <year>   -> <int>
 */

static int  read_date(Lexer*, Date&);
static void write_date(charArray*, Date);

void DateSet::write(charArray* output) const {
    normalize();

    rep->write(output);
    if (start != Date::First()) {
        append_string(output, "\nStart ");
        write_date(output, start);
    }

    if (finish != Date::Last()) {
        append_string(output, "\nFinish ");
        write_date(output, finish);
    }

    for (int i = 0; i < deleted.size(); i++) {
        append_string(output, "\nDeleted ");
        write_date(output, deleted[i]);
    }
    append_string(output, " End\n");
}

int DateSet::read(Lexer* lexer) {
    char const* keyword;

    if (! lexer->SkipWS() ||
        ! lexer->GetId(keyword)) {
        return 0;
    }

    Date date = Date::First();
    SmallIntSet set;
    set.Clear();

    DateSetRep* newRep;

    if (strcmp(keyword, "Empty") == 0) {
        newRep = new EmptyDateSetRep;
    }
    else if (strcmp(keyword, "Single") == 0) {
        newRep = new SingleDateSetRep(date);
    }
    else if (strcmp(keyword, "Days") == 0) {
        newRep = new DayBasedDateSetRep(1, date);
    }
    else if (strcmp(keyword, "Months") == 0) {
        newRep = new MonthDateSetRep(1, date);
    }
    else if (strcmp(keyword, "ComplexMonths") == 0) {
        newRep = new MonthBasedDateSetRep(1,1,date,ByDay,WeekDay::First(),0);
    }
    else if (strcmp(keyword, "WeekDays") == 0) {
        newRep = new WeekSetDateSetRep(set, set);
    }
    else if (strcmp(keyword, "MonthDays") == 0) {
        newRep = new MonthSetDateSetRep(set, set);
    }
    else {
        return 0;
    }

    if (! newRep->read(lexer)) {
        delete newRep;
        return 0;
    }

    delete rep;
    rep = newRep;
    normalized = 0;

    /* Read the rest of the spec */
    while (1) {
        char const* keyword;
        if (! lexer->SkipWS() ||
            ! lexer->GetId(keyword)) {
            return 0;
        }

        if (strcmp(keyword, "End") == 0) {
            return 1;
        }

        if (strcmp(keyword, "Start") == 0) {
            Date s;

            if (! lexer->SkipWS() ||
                ! read_date(lexer, s)) {
                return 0;
            }
            start = s;
            continue;
        }

        if (strcmp(keyword, "Finish") == 0) {
            Date f;

            if (! lexer->SkipWS() ||
                ! read_date(lexer, f)) {
                return 0;
            }
            finish = f;
            continue;
        }

        if (strcmp(keyword, "Deleted") == 0) {
            Date d;

            if (! lexer->SkipWS() ||
                ! read_date(lexer, d)) {
                return 0;
            }
            deleted.append(d);
            continue;
        }

        return 0;
    }

    return 1;
}

/*
 * Helper routines.
 */

/*
 * effects      Returns |x|
 */
static inline int ABS(int x) {
    return (x > 0) ? x : -x;
}

static int count_ranges(SmallIntSet set) {
    int in_range;
    int count;

    count = 0;
    in_range = 0;
    for (int i = 0; i <= SISetLargestMember; i++) {
        if (set.Member(i)) {
            if (! in_range) {
                /* Starting new range */
                count++;
                in_range = 1;
            }
        }
        else {
            if (in_range) {
                /* Ending range */
                in_range = 0;
            }
        }
    }

    return count;
}


/*
 * requires year(start) == year(finish)
 *          months contains exactly one contiguous range
 * modifies start, finish
 * effects  Clips start and finish to conform to month range.
 */
void set_month_range(Date& start, Date& finish, SmallIntSet months) {
    int year = start.GetYear();

    /* Find range of months */
    int a, b;

    /* Empty range to begin with */
    a = 12;
    b = 11;

    for (int i = 1; i <= 12; i++) {
        if (months.Member(i)) {
            a = i;
            while ((i <= 12) && months.Member(i)) {
                i++;
            }
            b = i-1;
            break;
        }
    }

    Month aMonth = Month::First() + a - 1;
    Month bMonth = Month::First() + b - 1;

    start = Date(1, aMonth, year);
    finish = Date(bMonth.Size(year), bMonth, year);
}

static int read_date(Lexer* lexer, Date& date) {
    int d, m, y;

    if (! lexer->SkipWS() ||
        ! lexer->GetNumber(d) ||
        ! lexer->Skip('/') ||
        ! lexer->GetNumber(m) ||
        ! lexer->Skip('/') ||
        ! lexer->GetNumber(y) ||
        (y < Year::First()) ||
        (y > Year::Last()) ||
        (m < 1) ||
        (m > 12) ||
        (d < 1) ||
        (d > (Month::First() + (m - 1)).Size(y))) {
        return 0;
    }

    date = Date(d, Month::First() + (m - 1), y);
    return 1;
}

static void write_date(charArray* output, Date date) {
    int d;
    Month m;
    WeekDay w;
    int y;

    date.BreakDown(d, w, m, y);
    format(output, "%d/%d/%d", d, m.Index(), y);
}

/*
 * Base operations.
 */

DateSetRep::DateSetRep() {
}

DateSetRep::~DateSetRep() {
}

int DateSetRep::search(Date anchor, Date& result,
                       Date start, Date finish, DateList const& deleted) const
{
    Date d = anchor;
    if (d < start) {
        d = start;
    }

    while (search(d, d) && (d <= finish)) {
        if (!contains_date(deleted, d)) {
            result = d;
            return 1;
        }
        d += 1;
    }
    return 0;
}

DateSetRep* DateSetRep::normalize(Date&, Date&, DateList&) const {
    return 0;
}

/*
 * Empty rep.
 */

EmptyDateSetRep::EmptyDateSetRep() {
}

EmptyDateSetRep::~EmptyDateSetRep() {
}

DateSetRep* EmptyDateSetRep::copy() const {
    return new EmptyDateSetRep;
}

DateSet::RepeatType EmptyDateSetRep::type() const {
    return DateSet::Other;
}

void EmptyDateSetRep::describe(charArray* buffer) const {
    append_string(buffer, "Empty");
}

void EmptyDateSetRep::describe_terse(charArray* buffer) const {
    append_string(buffer, "empty");
}

int EmptyDateSetRep::contains(Date) const {
    return 0;
}

int EmptyDateSetRep::search(Date, Date&) const {
    return 0;
}

int EmptyDateSetRep::read(Lexer* lexer) {
    return 1;
}

void EmptyDateSetRep::write(charArray* output) const {
    append_string(output, "Empty");
}

/*
 * SingleDateSetRep.
 */

SingleDateSetRep::SingleDateSetRep(Date d) {
    date = d;
    deleted = 0;
}

SingleDateSetRep::~SingleDateSetRep() {
}

DateSetRep* SingleDateSetRep::copy() const {
    return new SingleDateSetRep(date);
}

DateSet::RepeatType SingleDateSetRep::type() const {
    return DateSet::None;
}

void SingleDateSetRep::describe(charArray* buffer) const {
    append_date(buffer, date);
}

void SingleDateSetRep::describe_terse(charArray* buffer) const {
    format(buffer, "single");
}

int SingleDateSetRep::contains(Date d) const {
    return (!deleted && (d == date));
}

int SingleDateSetRep::search(Date d, Date& result) const {
    if (!deleted && (d <= date)) {
        result = date;
        return 1;
    }
    return 0;
}

int SingleDateSetRep::read(Lexer* lexer) {
    return (lexer->SkipWS() && read_date(lexer, date));
}

void SingleDateSetRep::write(charArray* output) const {
    append_string(output, "Single ");
    write_date(output, date);
}

/*
 * DayBasedDateSetRep.
 */

DayBasedDateSetRep::DayBasedDateSetRep(int i, Date a) {
    interval = i;
    anchor = a;
}

DayBasedDateSetRep::~DayBasedDateSetRep() {
}

DateSetRep* DayBasedDateSetRep::copy() const {
    return new DayBasedDateSetRep(interval, anchor);
}

DateSet::RepeatType DayBasedDateSetRep::type() const {
    switch (interval) {
      case 1:
        return DateSet::Daily;
      case 7:
        return DateSet::Weekly;
      case 14:
        return DateSet::BiWeekly;
      case 21:
        return DateSet::ThreeWeekly;
      case 28:
        return DateSet::FourWeekly;
      default:
        return DateSet::Other;
    }
}

void DayBasedDateSetRep::describe(charArray* buffer) const {
    switch (interval) {
      case 1:
        append_string(buffer, "Daily");
        break;
      case 7:
        append_string(buffer, "Every ");
        append_string(buffer, anchor.GetWDay().Name());
        break;
      case 14: case 21: case 28:
        append_string(buffer, anchor.GetWDay().Name());
        append_string(buffer, " Every ");
        append_int(buffer, (interval / 7));
        append_string(buffer, " Week");
        break;
      default:
        append_string(buffer, "Every ");
        append_int(buffer, interval);
        append_string(buffer, " Day From ");
        append_date(buffer, anchor);
        break;
    }
}

void DayBasedDateSetRep::describe_terse(charArray* buffer) const {
    format(buffer, "daily %d", interval);
}

int DayBasedDateSetRep::contains(Date d) const {
    int diff = ABS(d - anchor);
    return ((diff % interval) == 0);
}

int DayBasedDateSetRep::search(Date d, Date& result) const {
    /* Adjust d to match anchor */
    int diff = anchor - d;
    if (diff >= 0) {
        /* Anchor occurs on or after d */
        d += diff % interval;
    }
    else {
        /* Anchor occurs before d */
        d += interval - ((-1 - diff) % interval) - 1;
    }

    result = d;
    return 1;
}

int DayBasedDateSetRep::read(Lexer* lexer) {
    return (lexer->SkipWS() &&
            read_date(lexer, anchor) &&
            lexer->SkipWS() &&
            lexer->GetNumber(interval) &&
            (interval > 0));
}

void DayBasedDateSetRep::write(charArray* output) const {
    append_string(output, "Days ");
    write_date(output, anchor);
    format(output, " %d", interval);
}

/*
 * MonthBasedDateSetRep.
 */

MonthBasedDateSetRep::MonthBasedDateSetRep(int c, int i, Date a, MType t,
                                           WeekDay w, int b) {
    int junk1;
    WeekDay junk2;
    a.BreakDown(junk1, junk2, anchorMonth, anchorYear);
    anchor = a;
    count = c;
    interval = i;
    mtype = t;
    weekday = w;
    backward = b;
}

MonthBasedDateSetRep::~MonthBasedDateSetRep() {
}

DateSetRep* MonthBasedDateSetRep::copy() const {
    return new MonthBasedDateSetRep(count, interval, anchor, mtype,
                                    weekday, backward);
}

DateSet::RepeatType MonthBasedDateSetRep::type() const {
    switch (interval) {
      case 1:
        return DateSet::Monthly;
      case 2:
        return DateSet::TwoMonthly;
      case 3:
        return DateSet::ThreeMonthly;
      case 4:
        return DateSet::FourMonthly;
      case 6:
        return DateSet::SixMonthly;
      case 12:
        return DateSet::Annual;
      default:
        return DateSet::Other;
    }
}

void MonthBasedDateSetRep::describe(charArray* buffer) const {
    if (backward) {
        if (count != 1) {
            append_int(buffer, count);
            append_string(buffer, "-");
        }
        append_string(buffer, "Last");
    }
    else {
        append_int(buffer, count);
    }

    switch (mtype) {
      case ByDay:
        append_string(buffer, " of");
        break;
      case ByWorkDay:
        append_string(buffer, " Working Day");
        break;
      case ByWeek:
        append_string(buffer, " ");
        append_string(buffer, weekday.Name());
        break;
    }

    switch (interval) {
      case 1:
        append_string(buffer, " Every Month");
        break;
      case 12:
        append_string(buffer, " Every ");
        append_string(buffer, anchor.GetMonth().Name());
        break;
      default:
        append_string(buffer, " Every ");
        append_int(buffer, interval);
        append_string(buffer, " Month");
        break;
    }
}

void MonthBasedDateSetRep::describe_terse(charArray* buffer) const {
    int index;
    format(buffer, "monthly %d", interval);
    switch(mtype) {
      case ByDay:
        format(buffer, " day %d", backward ? -count : count);
        break;
      case ByWorkDay:
        format(buffer, " workday %d", backward ? -count : count);
        break;
      case ByWeek:
        format(buffer, " week %d %d", backward ? -count : count, weekday.Index());
        break;
      }
}

int MonthBasedDateSetRep::contains(Date d) const {
    int dDay, dYear;
    Month dMonth;
    WeekDay dWDay;

    d.BreakDown(dDay, dWDay, dMonth, dYear);

    if (find_in_month(dMonth, dYear) != dDay)
        return 0;

    int diff = ABS((dYear - anchorYear) * 12 + (dMonth - anchorMonth));
    return ((diff % interval) == 0);
}

int MonthBasedDateSetRep::search(Date d, Date& result) const {
    // Special checks for empty date set
    if (count < 1) return 0;
    switch (mtype) {
      case ByDay:
        if (count > 31) return 0;
        break;
      case ByWorkDay:
        if (count > 23) return 0;
        break;
      case ByWeek:
        if (count > 5) return 0;
        break;
    }

    int dDay, dYear;
    Month dMonth;
    WeekDay dWDay;

    d.BreakDown(dDay, dWDay, dMonth, dYear);

    /* Adjust <dMonth,dYear> to match anchor */
    if (dDay > find_in_month(dMonth, dYear))
        advance_months(dMonth, dYear, 1);

    /* Adjust based on interval */
    int diff = (anchorYear - dYear) * 12 + (anchorMonth - dMonth);
    if (diff > 0) {
        /* Anchor occurs after d */
        advance_months(dMonth, dYear, diff % interval);
    }
    else if (diff < 0) {
        /* Anchor occurs before d */
        advance_months(dMonth, dYear,
                       (interval - ((-1 - diff) % interval) - 1));
    }

    while (1) {
        int x = find_in_month(dMonth, dYear);
        if (x > 0) {
            result = Date(x, dMonth, dYear);
            return 1;
        }
        advance_months(dMonth, dYear, interval);
    }

    return 0;
}

void MonthBasedDateSetRep::write(charArray* output) const {
    format(output, "ComplexMonths %d %d ", interval, count);
    write_date(output, anchor);
    append_string(output, (backward ? " Backward " : " Forward "));
    switch (mtype) {
      case ByDay:
        append_string(output, "ByDay");
        break;
      case ByWorkDay:
        append_string(output, "ByWorkDay");
        break;
      case ByWeek:
        format(output, "ByWeek %d", weekday.Index());
        break;
    }
}

int MonthBasedDateSetRep::read(Lexer* lexer) {
    if (!(lexer->SkipWS() &&
          lexer->GetNumber(interval) &&
          lexer->SkipWS() &&
          lexer->GetNumber(count) &&
          lexer->SkipWS() &&
          read_date(lexer, anchor)))
        return 0;

    int junk1;
    WeekDay junk2;
    anchor.BreakDown(junk1, junk2, anchorMonth, anchorYear);

    char const* key;
    if (!(lexer->SkipWS() && lexer->GetId(key))) return 0;
    if (strcmp(key, "Backward") == 0)
        backward = 1;
    else if (strcmp(key, "Forward") == 0)
        backward = 0;
    else
        return 0;

    if (!(lexer->SkipWS() && lexer->GetId(key))) return 0;
    if (strcmp(key, "ByDay") == 0)
        mtype = ByDay;
    else if (strcmp(key, "ByWorkDay") == 0)
        mtype = ByWorkDay;
    else if (strcmp(key, "ByWeek") == 0) {
        int wday;
        if (!(lexer->SkipWS() && lexer->GetNumber(wday))) return 0;
        if ((wday < 1) || (wday > 7)) return 0;
        mtype = ByWeek;
        weekday = WeekDay::First() + (wday - 1);
    }
    else
        return 0;

    return 1;
}

// effects      Return occurrence for this set in specified "m/y".
//              Returns number < 1 if no such occurrence.

int MonthBasedDateSetRep::find_in_month(Month m, int y) const {
    int msize = m.Size(y);

    if (mtype == ByDay) {
        if ((count < 1) || (count > msize)) return 0;

        // Simple case?
        if (!backward) return count;

        // Backward search
        return (msize - count + 1);
    }

    WeekDay firstweekday = Date(1, m, y).GetWDay();

    if (mtype == ByWorkDay) {
        if (!backward) {
            WeekDay w = firstweekday;
            int x = 1;
            int left = count;

            while (x <= msize) {
                if ((w != WeekDay::Saturday()) && (w != WeekDay::Sunday()))
                    left--;
                if (left <= 0) return x;
                x++;
                w += 1;
            }
            return 0;
        }

        // Backward search
        int x = msize;
        WeekDay w = firstweekday + (msize - 1);
        int left = count;
        while (x > 0) {
            if ((w != WeekDay::Saturday()) && (w != WeekDay::Sunday()))
                left--;
            if (left <= 0) return x;
            x--;
            w -= 1;
        }
        return 0;
    }

    // By week...
    if (!backward) {
        int x = 1;
        WeekDay w = firstweekday;
        int left = count;

        // Advance to first occurrence of "weekday"
        while (w != weekday) {
            x++;
            w += 1;
        }

        // Now search
        while (x <= msize) {
            left--;
            if (left <= 0) return x;

            x += 7;
        }
        return 0;
    }

    int x = msize;
    WeekDay w = firstweekday + (msize - 1);
    int left = count;

    // Find last occurrence of "weekday"
    while (w != weekday) {
        x--;
        w -= 1;
    }

    // Now search
    while (x > 0) {
        left--;
        if (left <= 0) return x;
        x -= 7;
    }
    return 0;
}

MonthDateSetRep::MonthDateSetRep(int i, Date a)
    : MonthBasedDateSetRep(a.GetMDay(), i, a, ByDay, WeekDay::First(), 0)
{
}

DateSetRep* MonthDateSetRep::copy() const {
    return new MonthDateSetRep(interval, anchor);
}

int MonthDateSetRep::read(Lexer* lexer) {
    if (lexer->SkipWS() && read_date(lexer, anchor)) {
        WeekDay junk;
        anchor.BreakDown(count, junk, anchorMonth, anchorYear);

        mtype = ByDay;
        backward = 0;
        return (lexer->SkipWS() &&
                lexer->GetNumber(interval) &&
                (interval > 0));
    }
    return 0;
}

void MonthDateSetRep::write(charArray* output) const {
    assert(mtype == ByDay);
    assert(!backward);
    assert(anchor.GetMDay() == count);

    append_string(output, "Months ");
    write_date(output, anchor);
    format(output, " %d", interval);
}

/*
 * WeekSetDateSetRep.
 */

WeekSetDateSetRep::WeekSetDateSetRep(SmallIntSet d, SmallIntSet m) {
    days = d;
    months = m;
}

WeekSetDateSetRep::~WeekSetDateSetRep() {
}

DateSetRep* WeekSetDateSetRep::copy() const {
    return new WeekSetDateSetRep(days, months);
}

DateSet::RepeatType WeekSetDateSetRep::type() const {
    return DateSet::Other;
}

void WeekSetDateSetRep::describe(charArray* buffer) const {
    append_string(buffer, "Every");
    char const* sep = " ";
    for (int i = 1; i <= 7; i++) {
        if (days.Member(i)) {
            append_string(buffer, sep);
            append_string(buffer, (WeekDay::First() + (i-1)).Name());
            sep = ", ";
        }
    }
}

void WeekSetDateSetRep::describe_terse(charArray* buffer) const {
    format(buffer, "weekly {");
    for (int i = 1; i <= 7; i++) {
        if (days.Member(i))
            format(buffer, " %d", i);
    }
    append_string(buffer, " }");
}

int WeekSetDateSetRep::contains(Date date) const {
    int d, y;
    Month m;
    WeekDay w;
    date.BreakDown(d, w, m, y);

    return (days.Member(w.Index()) && months.Member(m.Index()));
}

int WeekSetDateSetRep::search(Date date, Date& result) const {
    int d, y;
    Month m;
    WeekDay wd;

    date.BreakDown(d, wd, m, y);

    int last = Year::Last();
    while (y <= last) {
        /* Search in month m */
        int mSize = m.Size(y);

        if (months.Member(m.Index())) {
            /* Search for day match */
            while (d <= mSize) {
                if (days.Member(wd.Index())) {
                    Date xdate = Date(d, m, y);
                    result = xdate;
                    return 1;
                }
                d++;
                wd += 1;
            }
        }

        /* Advance to next month */
        if (m == Month::Last()) {
            m = Month::First();
            y++;
        }
        else {
            m += 1;
        }

        wd += mSize - d + 1;
        d = 1;
    }

    /* No occurrence */
    return 0;
}

int WeekSetDateSetRep::read(Lexer* lexer) {
    char const* keyword;

    return (lexer->SkipWS() &&
            days.Read(lexer) &&
            lexer->SkipWS() &&
            lexer->GetId(keyword) &&
            (strcmp(keyword, "Months") == 0) &&
            lexer->SkipWS() &&
            months.Read(lexer));
}

void WeekSetDateSetRep::write(charArray* output) const {
    append_string(output, "WeekDays ");
    days.Write(output);
    append_string(output, " Months ");
    months.Write(output);
}

DateSetRep* WeekSetDateSetRep::normalize(Date& start, Date& finish,
                                         DateList& deleted) const {
    Date firstDate;

    if (! DateSetRep::search(Date::First(),firstDate,start,finish,deleted)) {
        return new EmptyDateSetRep;
    }

    int years = finish.GetYear() - start.GetYear() + 1;

    if ((months.Size() == 0) || (days.Size() == 0)) {
        /* Empty */
        return new EmptyDateSetRep;
    }

    if (days.Size() == 1) {
        if (months.Size() == 12) {
            /* Weekly */
            return new DayBasedDateSetRep(7, firstDate);
        }
        
        if ((years == 1) && (count_ranges(months) == 1)) {
            /* Weekly in range */
            set_month_range(start, finish, months);
            return new DayBasedDateSetRep(7, firstDate);
        }
    }
    
    if (days.Size() == 7) {
        if (months.Size() == 12) {
            /* Daily */
            return new DayBasedDateSetRep(1, firstDate);
        }
        
        if ((years == 1) && (count_ranges(months) == 1)) {
            /* Daily in range */
            set_month_range(start, finish, months);
            return new DayBasedDateSetRep(1, firstDate);
        }
    }

    return 0;
}

/*
 * MonthSetDateSetRep.
 */

MonthSetDateSetRep::MonthSetDateSetRep(SmallIntSet d, SmallIntSet m) {
    days = d;
    months = m;
}

MonthSetDateSetRep::~MonthSetDateSetRep() {
}

DateSetRep* MonthSetDateSetRep::copy() const {
    return new MonthSetDateSetRep(days, months);
}

DateSet::RepeatType MonthSetDateSetRep::type() const {
    return DateSet::Other;
}

void MonthSetDateSetRep::describe(charArray* buffer) const {
    append_string(buffer, "Complex Monthly Repetition");
}

void MonthSetDateSetRep::describe_terse(charArray* buffer) const {
    append_string(buffer, "monthset {");
    for (int i = 1; i <= 12; i++) {
        if (months.Member(i))
            format(buffer, " %d", i);
    }
    append_string(buffer, " } {");
    for (int i = 1; i <= 31; i++) {
        if (days.Member(i))
            format(buffer, " %d", i);
    }
    append_string(buffer, "}");
}

int MonthSetDateSetRep::contains(Date date) const {
    int d, y;
    Month m;
    WeekDay w;
    date.BreakDown(d, w, m, y);

    return (days.Member(d) && months.Member(m.Index()));
}

int MonthSetDateSetRep::search(Date date, Date& result) const {
    int d, y;
    Month m;
    WeekDay wd;

    date.BreakDown(d, wd, m, y);

    int last = Year::Last();
    while (y <= last) {
        /* Search in month m */
        int mSize = m.Size(y);

        if (months.Member(m.Index())) {
            /* Search for day match */
            while (d <= mSize) {
                if (days.Member(d)) {
                    result = Date(d, m, y);
                    return 1;
                }
                d++;
            }
        }

        /* Advance to next month */
        if (m == Month::Last()) {
            m = Month::First();
            y++;
        }
        else {
            m += 1;
        }

        d = 1;
    }

    /* No occurrence */
    return 0;
}

int MonthSetDateSetRep::read(Lexer* lexer) {
    char const* keyword;

    return (lexer->SkipWS() &&
            days.Read(lexer) &&
            lexer->SkipWS() &&
            lexer->GetId(keyword) &&
            (strcmp(keyword, "Months") == 0) &&
            lexer->SkipWS() &&
            months.Read(lexer));
}

void MonthSetDateSetRep::write(charArray* output) const {
    append_string(output, "MonthDays ");
    days.Write(output);
    append_string(output, " Months ");
    months.Write(output);
}

DateSetRep* MonthSetDateSetRep::normalize(Date& start, Date& finish,
                                         DateList& deleted) const {
    Date firstDate;

    if (! DateSetRep::search(Date::First(),firstDate,start,finish,deleted)) {
        return new EmptyDateSetRep;
    }

    int years = finish.GetYear() - start.GetYear() + 1;

    if ((months.Size() == 0) || (days.Size() == 0)) {
        /* Empty */
        return new EmptyDateSetRep;
    }

    if (days.Size() == 1) {
        if (months.Size() == 1) {
            /* Annual */
            return new MonthDateSetRep(12, firstDate);
        }
        
        if (months.Size() == 12) {
            /* Monthly */
            return new MonthDateSetRep(1, firstDate);
        }
        
        if ((years == 1) && (count_ranges(months) == 1)) {
            /* Monthly in range */
            set_month_range(start, finish, months);
            return new MonthDateSetRep(1, firstDate);
        }
    }
    
    else if (days.Size() == 31) {
        if (months.Size() == 12) {
            /* Daily */
            return new DayBasedDateSetRep(1, firstDate);
        }
        
        if ((years == 1) && (count_ranges(months) == 1)) {
            /* Daily in range */
            set_month_range(start, finish, months);
            return new DayBasedDateSetRep(1, firstDate);
        }
    }
    
    else if ((years==1) && (months.Size()==1) && (count_ranges(days)==1)) {
        /* Find first and last days in range */
        int i;
        int a = 1;
        int b = 0;
        for (i = 1; i <= 31; i++) {
            if (days.Member(i)) {
                a = i;
                while ((i <= 31) && days.Member(i)) {
                    i++;
                }
                b = i - 1;
                break;
            }
        }
        
        /* Find month */
        Month m = Month::First();   /* Should not be necessary */
        for (i = 1; i <= 12; i++) {
            if (months.Member(i)) {
                m = Month::First() + i - 1;
                break;
            }
        }
        
        /* Find new range */
        int year = start.GetYear();
        Date newStart = Date(a, m, year);
        Date newFinish = Date(b, m, year);

        /* Daily with range */
        start = newStart;
        finish = newFinish;
        return new DayBasedDateSetRep(1, firstDate);
    }

    return 0;
}

implementArray(DateList,Date)

static void advance_months(Month& month, int& year, int delta) {
    /* Advance by years */
    year += (delta / 12);
    delta = delta % 12;

    /* Advance remaining months */
    if ((Month::Last() - month) < delta) {
        /* Have to advance to next year */
        delta -= (Month::Last() - month) + 1;
        year++;
        month = Month::First();
    }

    month += delta;
}
