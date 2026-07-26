/* Copyright (c) 1993 by Sanjay Ghemawat */
#ifndef _DATESET_H
#define _DATESET_H

#include "basic.h"
#include "Array.h"
#include "Date.h"
#include "arrays.h"
#include "smallintset.h"

declareArray(DateList,Date)

class DateSetRep;
class Lexer;
class charArray;
/*
 * A date set is a mutable set of dates.
 */

class DateSet {
  public:
    /*
     * effects  post(*this) = empty
     */
    DateSet();

    /*
     * effects  post(*this) = d
     */
    DateSet(DateSet const& d);

    ~DateSet();

    /*
     * modifies *this
     * effects  post(*this) = d
     */
    void operator = (DateSet const& d);

    /*
     * effects  Returns true iff d is in *this.
     */
    int contains(Date d) const;

    /*
     * effects  Returns true iff *this has more than one element.
     */
    int repeats() const;

    /*
     * effects  Returns true iff *this has zero elements
     */
    int empty() const;

    int get_range(Date&, Date&) const;
    // Return date clipping range

    /*
     * modifies result
     * effects  If *this is not empty, sets result to the smallest element in
     *          *this and returns true.  Else returns false.
     */
    int  first(Date& result) const;

    /*
     * modifies result
     * effects  If *this contains a date greater than d, then sets
     *          result to the smallest such date and returns true.
     *          Else returns false.
     */
    int next(Date d, Date& result) const;

    /*
     * effects  Returns true iff first occurrence of s1 occurs strictly
     *          before the first occurrence of s2.  If a set is empty,
     *          its first occurrence is assumed to occur after all legal
     *          dates.
     */
    static int occurs_before(DateSet const& s1, DateSet const& s2);

    /*
     * effects  Returns -1, 0, +1 depending on whether the first
     *          occurrence of s1 occurs strictly before, on the same date
     *          as, or strictly after, the first occurrence of s2.
     *          Empty sets are treated as in occurs_before.
     */
    static int occurs_compare(DateSet const& s1, DateSet const& s2);

    /*
     * Repetition type.
     */
    enum RepeatType {
        None,
        Daily,
        Weekly,
        BiWeekly,
        ThreeWeekly,
        FourWeekly,
        Monthly,
        TwoMonthly,
        ThreeMonthly,
        FourMonthly,
        SixMonthly,
        Annual,
        Other
        };

    /*
     * effects  Returns description of set contents.
     */
    RepeatType type() const;

    /*
     * modifies "buffer".
     * effects  Append brief item description to "buffer".
     *          No "null" character is stored in "buffer".
     */
    void describe(charArray* buffer) const;

    /*
     * modifies "buffer".
     * effects  Append brief but complete item description to "buffer".
     *          No "null" character is stored in "buffer".
     *
     */
    void describe_terse(charArray* buffer) const;

    /*
     * modifies *this
     * effects  post(*this) = the empty set
     */
    void set_empty();

    /*
     * modifies *this
     * effects  post(*this) = { d }
     */
    void set_date(Date d);

    /*
     * requires interval > 0
     * modifies *this
     * effects  post(*this) = { d | d = (anchor + k*interval) }
     *                                  where k is an integer.
     */
    void set_day_based_repeat(int interval, Date anchor);

    /*
     * requires interval > 0
     * modifies *this
     * effects  post(*this) = { d | d is an integral multiple of <interval>
     *                              months away from <anchor> }
     */
    void set_month_based_repeat(int interval, Date anchor);

    /*
     * modifies *this
     * effects  post(*this) = { d | weekday(d) in days and month(d) in months }
     */
    void set_week_set(SmallIntSet days, SmallIntSet months);
    
    /*
     * modifies *this
     * effects  post(*this) = { d | monthday(d) in days and
     *                              month(d) in months }
     */
    void set_month_set(SmallIntSet days, SmallIntSet months);

    /* Complex monthly repetition */

    /*
     * requires count > 0, interval > 0
     * modifies *this
     * effects  If "!backward"
     *                  post(*this) = { d | monthday(d) == count and
     *                                      month(d) is an integral multiple of
     *                                      <interval> months away from
     *                                      month(<anchor>) }
     *          Else
     *                  post(*this) = { d | monthday(d) ==
     *                                      monthsize(d) - count + 1 and
     *                                      month(d) is an integral multiple of
     *                                      <interval> months away from
     *                                      month(<anchor>) }
     *
     * Example:
     *
     * Second-last day of every month
     *          set_monthly_by_days(2, 1, Date(...), true);
     */
    void set_monthly_by_days(int count, int interval, Date anchor,
                             int backward);

    /*
     * requires count > 0, interval > 0
     * modifies *this
     * effects  Just like "set_monthly_by_days" except that "count"
     *          identifies the number of working days from the beginning
     *          or end of the month.
     *
     * Examples:
     *
     * Fourth-last work day of every month:
     *          set_monthly_by_workdays(4, 1, Date(...), true);
     */
    void set_monthly_by_workdays(int count, int interval, Date anchor,
                                 int backward);

    /*
     * requires count > 0, interval > 0
     * modifies *this
     * effects  Just like "set_monthly_by_days" except that instead
     *          of "count" identifying the number of days from beginning
     *          or end of the month, it identifies the number of weeks
     *          from the beginning or edn.
     *
     * Examples:
     *
     * Third Thursday of every month:
     *          set_monthly_by_weeks(3, 1, Date(...), Thursday);
     *
     * Last Friday of January:
     *          set_monthly_by_weeks(1, 12, Friday, Date(..,January,..), true);
     */
    void set_monthly_by_weeks(int count, WeekDay w, int interval, Date anchor,
                              int backward);

    /*
     * modifies *this
     * effects  post(*this) = pre(*this) - { d | d < start }
     */
    void set_start(Date s) { start = s; normalized = 0; }

    /*
     * modifies *this
     * effects  post(*this) = pre(*this) - { d | d > finish }
     */
    void set_finish(Date f) { finish = f; normalized = 0; }

    /*
     * modifies *this
     * effects  post(*this) = pre(*this) - { d }
     */
    void delete_occurrence(Date d) { deleted.append(d); normalized = 0; }

    /*
     * modifies *this
     * effects  post(*this) = string representation of dateset read from lexer.
     *          Returns true iff successful.  If not successful, *this may
     *          have arbitrary contents.
     */
    int read(Lexer* lexer);

    /*
     * effects  Appendsa string representation of *this to buffer.
     */
    void write(charArray* buffer) const;
  private:
    DateSetRep* rep;            /* Type specific representation */
    int         normalized;     /* Rep has been normalized? */
    DateList    deleted;        /* List of deleted dates */

    /* Range limit on dates */
    Date        start;
    Date        finish;

    /*
     * Abstraction Function
     *
     * A(this) = { d | d is in A(rep) and d is not in deleted and
     *                 d >= start and d <= finish }.
     */

    /*
     * Normalize representation.
     */
    void normalize() const;
};

#endif /* _DATESET_H */
