/* Copyright (c) 1993 Sanjay Ghemawat */
/*
 * DateSet tests.
 *
 * Conversion tests
 */

// XXX The following types of datesets are incompletely tested.
//
// 1. Monthly by days
// 2. Monthly by work days
// 3. Monthly by week

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Month.h"
#include "WeekDay.h"
#include "dateset.h"
#include "lexer.h"

static void t_empty();
static void t_single();
static void t_days();
static void t_months();
static void t_months_by_day();
static void t_months_by_workday();
static void t_months_by_week();
static void t_old();
static void t_convert();
static void t_io();

int
main() {
    t_empty();
    t_single();
    t_days();
    t_months();
    t_months_by_day();
    t_months_by_workday();
    t_months_by_week();
    t_old();
    t_convert();
    t_io();
    return 0;
}

/* Compare contents of datesets. */
static void compare(DateSet const& d1, DateSet const& d2) {
    assert(d1.repeats() == d2.repeats());
    assert(d1.empty() == d2.empty());

    if (d1.empty()) {
        return;
    }

    Date x1, x2;
    d1.first(x1);
    d2.first(x2);

    /* Check at most 100 occurrences */
    for (int count = 0; count < 100; count++) {
        assert(x1 == x2);

        int result = d1.next(x1, x1);
        assert(result == d2.next(x2, x2));
        if (! result) {
            break;
        }
    }
}

/* Perform IO checks. */
static void io_check1 (DateSet const& d) {
    charArray out;
    d.write(&out);
    out.append('\n');
    out.append('\0');

    Lexer lex = &out;
    DateSet d2;
    assert(d2.read(&lex));

    compare(d, d2);
}

static void io_check(DateSet const& d) {
    DateSet d2 = d;

    io_check1(d2);

    d2.set_start(Date::First()+1000);
    io_check1(d2);

    d2.set_finish(Date::Last()-1000);
    io_check1(d2);
}

/* Return number of months difference between d1 and d2 */
static int months_diff(Date d1, Date d2) {
    int day1, day2;
    Month m1, m2;
    int y1, y2;
    WeekDay w1, w2;

    d1.BreakDown(day1, w1, m1, y1);
    d2.BreakDown(day2, w2, m2, y2);

    return ((y1 - y2) * 12) + (m1 - m2);
}

// Test empty sets
static void t_empty() {
    DateSet d;
    Date tmp;

    printf("test empty\n");

    assert(d.empty());
    assert(! d.repeats());
    assert(! d.first(tmp));
    assert(! d.next(Date::First(), tmp));
    assert(d.type() == DateSet::Other);

    /* Delete test */
    {
        DateSet d2;

        d2 = d;
        d2.delete_occurrence(Date(1, Month::January(), 1980));
        compare(d, d2);
    }
}

// Test singleton sets
static void t_single() {
    DateSet empty;
    Date date(1, Month::January(), 1980);
    Date tmp;

    printf("test single\n");

    for (int i = 0; i < 100; i++) {
        DateSet d;
        d.set_date(date);

        assert(d.contains(date));
        assert(! d.repeats());
        assert(! d.empty());
        assert(d.first(tmp) && (tmp == date));
        assert(! d.next(date, tmp));
        assert(d.type() == DateSet::None);

        DateSet d2 = d;
        compare(d2, d);
        d2.set_start(date);
        compare(d2, d);
        d2.set_start(date+1);
        compare(d2, empty);

        d2 = d;
        compare(d2, d);
        d2.set_finish(date);
        compare(d2, d);
        d2.set_finish(date-1);
        compare(d2,empty);

        d2 = d;
        d2.delete_occurrence(date-1);
        compare(d2, d);
        d2.delete_occurrence(date+1);
        compare(d2, d);
        d2.delete_occurrence(date);
        compare(d2, empty);

        d2 = d;
        d2.delete_occurrence(date);
        compare(d2, empty);

        date += 1;
    }
}

static void t_days() {
    DateSet empty;
    Date init(1, Month::January(), 1980);
    Date tmp;

    printf("test day based repeat\n");
    for (int interval = 1; interval <= 10; interval++) {
        printf("   interval = %d\n", interval);
        for (int i = 0; i < 200; i += 17) {
            /* Test for a number of anchors */
            Date date = init + i;

            DateSet d, d2, d3;
            d.set_day_based_repeat(interval, date);

            assert(d.contains(date));
            assert(d.repeats());
            assert(! d.empty());
            switch(interval) {
              case 1:
                assert(d.type() == DateSet::Daily);
                break;
              case 7:
                assert(d.type() == DateSet::Weekly);
                break;
              case 14:
                assert(d.type() == DateSet::BiWeekly);
                break;
              case 21:
                assert(d.type() == DateSet::ThreeWeekly);
                break;
              case 28:
                assert(d.type() == DateSet::FourWeekly);
                break;
              default:
                assert(d.type() == DateSet::Other);
                break;
            }

            assert(d.first(tmp) && (((date - tmp) % interval) == 0));
            assert(d.next(tmp, tmp) && (((date - tmp) % interval) == 0));
            assert(d.next(tmp, tmp) && (((date - tmp) % interval) == 0));
            assert(d.next(tmp, tmp) && (((date - tmp) % interval) == 0));
            assert(d.next(tmp, tmp) && (((date - tmp) % interval) == 0));

            d2.set_day_based_repeat(interval, date+interval);
            compare(d, d2);

            d2.set_day_based_repeat(interval, date-interval);
            compare(d, d2);

            /* Search starts on or after anchor */
            int j;
            for (j = 0; j < interval; j++) {
                Date start = date + j;
                assert(d.next(start, tmp) && (tmp == (date + interval)));
            }
            assert(d.next(date+interval, tmp) && (tmp == (date + 2*interval)));

            /* Search starts before anchor */
            for (j = 1; j <= interval; j++) {
                Date start = date - j;
                assert(d.next(start, tmp) && (tmp == date));
            }

            /* Range checking */
            for (int start = -4; start <= 4; start++) {
                for (int finish = -4; finish <= 4; finish++) {
                    d2 = d;
                    d2.set_start(date + start);
                    d2.set_finish(date + finish);

                    /* Delete test */
                    d3 = d2;
                    d3.delete_occurrence(date+start-1);
                    compare(d2, d3);
                    d3.delete_occurrence(date+finish+1);
                    compare(d2, d3);
                    d3.delete_occurrence(date);
                    assert(! d3.contains(date));
                    assert(!d3.next(date-1,tmp) || (tmp != date));

                    tmp = Date::First();
                    while (d2.next(tmp, tmp)) {
                        assert(tmp >= date + start);
                        assert(tmp <= date + finish);

                        if (tmp < date) {
                            assert(((date - tmp) % interval) == 0);
                        }
                        else {
                            assert(((tmp - date) % interval) == 0);
                        }
                    }
                }
            }
        }
    }
}

static void t_months() {
    DateSet empty;
    Date init(1, Month::January(), 1980);

    printf("test month based repeat\n");


    for (int interval = 1; interval <= 20; interval++) {
        printf("   interval = %d\n", interval);
        for (int mday = 1; mday <= 28; mday += 7) {
            Date date = Date(mday, Month::January(), 1980);

            Date tmp;
            DateSet d, d2, d3;
            d.set_month_based_repeat(interval, date);

            assert(d.contains(date));
            assert(d.repeats());
            assert(! d.empty());
            switch (interval) {
              case 1:
                assert(d.type() == DateSet::Monthly);
                break;
              case 2:
                assert(d.type() == DateSet::TwoMonthly);
                break;
              case 3:
                assert(d.type() == DateSet::ThreeMonthly);
                break;
              case 4:
                assert(d.type() == DateSet::FourMonthly);
                break;
              case 6:
                assert(d.type() == DateSet::SixMonthly);
                break;
              case 12:
                assert(d.type() == DateSet::Annual);
                break;
              default:
                assert(d.type() == DateSet::Other);
                break;
            }

            assert(d.first(tmp) && ((months_diff(date, tmp) % interval) == 0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));

            assert(d.next(date,tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));
            assert(d.next(tmp, tmp) && ((months_diff(date,tmp)%interval)==0));

            d.first(tmp);
            d2.set_month_based_repeat(interval, tmp);
            compare(d, d2);

            d.next(date, tmp);
            d2.set_month_based_repeat(interval, tmp);
            compare(d, d2);

            /* Search starts on or after anchor */
            int j;
            for (j = 0; j < interval; j++) {
                Date start = date + j*31;
                assert(d.next(start, tmp) && ((months_diff(date,tmp)%interval) == 0));
            }

            /* Search starts before anchor */
            for (j = 1; j <= interval; j++) {
                Date start = date - j;
                assert(d.next(start, tmp) && ((months_diff(tmp,date)%interval) == 0));
            }

            /* Range checking */
            for (int start = -4; start <= 4; start++) {
                for (int finish = -4; finish <= 4; finish++) {
                    d2 = d;
                    d2.set_start(date + start);
                    d2.set_finish(date + finish);

                    /* Delete test */
                    d3 = d2;
                    d3.delete_occurrence(date+start-1);
                    compare(d2, d3);
                    d3.delete_occurrence(date+finish+1);
                    compare(d2, d3);
                    d3.delete_occurrence(date);
                    assert(! d3.contains(date));
                    assert(!d3.next(date-1,tmp) || (tmp != date));

                    tmp = Date::First();
                    while (d2.next(tmp, tmp)) {
                        assert(tmp >= date + start);
                        assert(tmp <= date + finish);

                        if (tmp < date) {
                            assert((months_diff(date,tmp) % interval) == 0);
                        }
                        else {
                            assert((months_diff(tmp,date) % interval) == 0);
                        }
                    }
                }
            }
        }
    }

    /* Check month size idiosyncracies */
    {
        DateSet d;
        Date tmp;
        Month feb = Month::February();

        d.set_month_based_repeat(12, Date(29, feb, 1980));

        assert(d.next(init, tmp) && (tmp == Date(29, feb, 1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, feb, 1984)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, feb, 1988)));

        d.set_month_based_repeat(1, Date(29, feb, 1980));

        assert(d.next(init, tmp) && (tmp == Date(29, Month::January(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::February(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::March(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::April(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::May(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::June(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::July(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::August(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::September(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::October(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::November(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::December(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::January(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(29, Month::March(),1981)));

        d.set_month_based_repeat(1, Date(31, Month::January(), 1980));

        assert(d.next(init, tmp) && (tmp == Date(31, Month::January(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::March(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::May(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::July(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::August(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::October(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::December(),1980)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::January(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::March(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::May(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::July(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::August(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::October(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::December(),1981)));
        assert(d.next(tmp, tmp) && (tmp == Date(31, Month::January(),1982)));
    }
}

static void t_months_by_day() {
    Month m = Month::January();
    int year = 1980;
   
    printf("test month repeat by days\n");

    // Check a whole bunch of months
    for (int mcount = 0; mcount < 50; mcount++) {
        int msize = m.Size(year);

        for (int i = 1; i <= msize; i++) {
            Date date, tmp;
            DateSet d;
            d.set_monthly_by_days(i, 1, Date(1, m, year), 0);

            date = Date(i, m, year);
            assert(d.contains(date));
            if (i > 1) {
                assert(d.next(date-1, tmp) && (tmp == date));
            }

            assert(d.next(date, tmp) && (tmp > Date(msize, m, year)));
        }

        // Move to next month
        if (m == Month::December()) {
            m = Month::January();
            year++;
        }
        else {
            m += 1;
        }
    }
}

static void t_months_by_workday() {
    Month m = Month::January();
    int year = 1980;
   
    printf("test month repeat by work days\n");

    // Check a whole bunch of months
    for (int mcount = 0; mcount < 50; mcount++) {
        int msize = m.Size(year);

        for (int i = 1; i <= msize; i++) {
            Date date, tmp;
            DateSet d;
            d.set_monthly_by_workdays(i, 1, Date(1, m, year), 0);

            if (i > 23) {
                assert(d.empty());
                continue;
            }

            date = Date(i, m, year);
            for (int j = 0; j < 20; j++) {
                assert(d.next(date, tmp));
                WeekDay w = tmp.GetWDay();
                assert((w != WeekDay::Saturday()) && (w != WeekDay::Sunday()));
                date = tmp;
            }
        }

        // Move to next month
        if (m == Month::December()) {
            m = Month::January();
            year++;
        }
        else {
            m += 1;
        }
    }
}

static void t_months_by_week() {
}

static void t_io() {
    Date date(1, Month::January(), 1980);
    DateSet d;
    int i, j, k;

    printf("test io\n");

    d.set_empty();
    io_check(d);

    d.set_date(date);
    io_check(d);

    for (i = 1; i < 4; i++) {
        d.set_day_based_repeat(i, date);
        io_check(d);

        DateSet d2;

        d2 = d;
        d2.set_start(date-1);
        io_check(d2);

        d2 = d;
        d2.set_finish(date+1);
        io_check(d2);

        for (j = -2; j <= 2; j++) {
            for (k = -2; k <= 2; k++) {
                d2 = d;
                d2.set_start(date-k);
                d2.set_finish(date+k);
                io_check(d2);
            }
        }
    }

    for (i = 1; i < 4; i++) {
        d.set_month_based_repeat(i, date);
        io_check(d);

        DateSet d2;

        d2 = d;
        d2.set_start(date-1);
        io_check(d2);

        d2 = d;
        d2.set_finish(date+1);
        io_check(d2);

        for (j = -2; j <= 2; j++) {
            for (k = -2; k <= 2; k++) {
                d2 = d;
                d2.set_start(date-k);
                d2.set_finish(date+k);
                io_check(d2);
            }
        }
    }
}

/*
 * requires     0 <= min <= max <= SISetLargestMember
 * modifies     set
 * effects      post(set) = { x | (min <= x) and (x <= max) with 50% prob. }
 */
static void randomize(SmallIntSet& set, int min, int max) {
    unsigned long int value = random();

    set.Clear();
    for (unsigned int i = min; i <= max; i++) {
        if (value & (1 << i)) {
            set.Insert(i);
        }
    }
}

static void old_mcheck(DateSet const& d,
                       SmallIntSet days,
                       SmallIntSet months,
                       Date startDate,
                       Date firstDate,
                       Date lastDate) {
    int count = 0;
    Date tmp = startDate;
    while (d.next(tmp, tmp)) {
        int mday, year;
        Month month;
        WeekDay wday;
        
        assert((tmp >= firstDate) && (tmp <= lastDate));
        assert(d.contains(tmp));
        tmp.BreakDown(mday, wday, month, year);
        assert(days.Member(mday) && months.Member(month.Index()));

        Date tmp2;
        DateSet d2 = d;

        d2.delete_occurrence(tmp-1);
        assert(d2.contains(tmp));
        assert(d2.next(tmp-1,tmp2) && (tmp2 == tmp));
        d2.delete_occurrence(tmp+1);
        assert(d2.contains(tmp));
        assert(d2.next(tmp-1,tmp2) && (tmp2 == tmp));
        d2.delete_occurrence(tmp);
        assert(!d2.contains(tmp));
        assert(!d2.next(tmp-1,tmp2) || (tmp2 != tmp));
        
        count++;
        if (count > 20) {
            break;
        }
    }
}

static void old_wcheck(DateSet const& d,
                       SmallIntSet days,
                       SmallIntSet months,
                       Date startDate,
                       Date firstDate,
                       Date lastDate) {
    int count = 0;
    Date tmp = startDate;
    while (d.next(tmp, tmp)) {
        int mday, year;
        Month month;
        WeekDay wday;
        
        assert((tmp >= firstDate) && (tmp <= lastDate));
        assert(d.contains(tmp));
        tmp.BreakDown(mday, wday, month, year);
        assert(days.Member(wday.Index()) && months.Member(month.Index()));

        Date tmp2;
        DateSet d2 = d;

        d2.delete_occurrence(tmp-1);
        assert(d2.contains(tmp));
        assert(d2.next(tmp-1,tmp2) && (tmp2 == tmp));
        d2.delete_occurrence(tmp+1);
        assert(d2.contains(tmp));
        assert(d2.next(tmp-1,tmp2) && (tmp2 == tmp));
        d2.delete_occurrence(tmp);
        assert(!d2.contains(tmp));
        assert(!d2.next(tmp-1,tmp2) || (tmp2 != tmp));
        
        count++;
        if (count > 20) {
            break;
        }
    }
}

static void t_old() {
    DateSet d;
    Date firstDate = Date::First();
    Date lastDate = Date::Last();
    Date startDate = Date(1, Month::January(), 1980);
    Date finishDate = Date(31, Month::December(), 1982);
    int i;

    printf("test old format\n");

    /* Test monthly repetition */
    for (i = 0; i < 100; i++) {
        SmallIntSet days, months;
        randomize(days, 1, 31);
        randomize(months, 1, 12);

        d.set_month_set(days, months);

        old_mcheck(d, days, months, firstDate, firstDate, lastDate);
        if ((i % 29) == 0) io_check(d);

        DateSet d2 = d;
        d2.delete_occurrence(startDate);
        if ((i % 29) == 0) io_check(d2);
        d2.delete_occurrence(finishDate);
        if ((i % 29) == 0) io_check(d2);

        d.set_start(startDate);
        d.set_finish(finishDate);
        old_mcheck(d, days, months, startDate - 20, startDate, finishDate);
        if ((i % 29) == 0) io_check(d2);
    }

    /* Test weekly repetition */
    for (i = 0; i < 100; i++) {
        SmallIntSet days, months;
        randomize(days, 1, 7);
        randomize(months, 1, 12);

        d.set_week_set(days, months);

        old_wcheck(d, days, months, firstDate, firstDate, lastDate);
        if ((i % 29) == 0) io_check(d);

        DateSet d2 = d;
        d2.delete_occurrence(startDate);
        if ((i % 29) == 0) io_check(d2);
        d2.delete_occurrence(finishDate);
        if ((i % 29) == 0) io_check(d2);

        d.set_start(startDate);
        d.set_finish(finishDate);
        old_wcheck(d, days, months, startDate - 20, startDate, finishDate);
        if ((i % 29) == 0) io_check(d);
    }
}

static void t_convert() {
    SmallIntSet emptySet;
    SmallIntSet singleSet;
    SmallIntSet allMonths;
    SmallIntSet rangeSet;
    SmallIntSet allWeekDays;
    SmallIntSet allMonthDays;
    DateSet d;
    Date tmp;
    int i;

    printf("test conversion\n");

    emptySet.Clear();
    singleSet.Clear();
    singleSet.Insert(5);

    allMonths.Clear();
    for (i = 1; i <= 12; i++) {
        allMonths.Insert(i);
    }

    rangeSet.Clear();
    for (i = 3; i <= 6; i++) {
        rangeSet.Insert(i);
    }

    allWeekDays.Clear();
    for (i = 1; i <= 7; i++) {
        allWeekDays.Insert(i);
    }

    allMonthDays.Clear();
    for (i = 1; i <= 31; i++) {
        allMonthDays.Insert(i);
    }

    {
        d.set_month_set(emptySet, singleSet);
        assert(d.empty());

        d.set_week_set(emptySet, singleSet);
        assert(d.empty());

        d.set_month_set(singleSet, singleSet);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1979));
        assert(d.empty());
    }

    {
        d.set_week_set(singleSet, allMonths);
        assert(d.type() == DateSet::Weekly);

        d.set_week_set(singleSet, allMonths);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1990));
        assert(d.type() == DateSet::Weekly);

        assert(d.first(tmp) && (tmp >= Date(1, Month::January(), 1980)));
    }

    {
        d.set_week_set(singleSet, rangeSet);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1980));
        assert(d.type() == DateSet::Weekly);

        assert(d.first(tmp) && (tmp >= Date(1, Month::January(), 1980)));
    }

    {
        d.set_week_set(allWeekDays, allMonths);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1980));
        assert(d.type() == DateSet::Daily);

        assert(d.first(tmp) && (tmp >= Date(1, Month::January(), 1980)));

        d.set_week_set(allWeekDays, rangeSet);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1980));
        assert(d.type() == DateSet::Daily);

        assert(d.first(tmp) && (tmp >= Date(1, Month::January(), 1980)));
    }

    {
        d.set_month_set(singleSet, singleSet);
        assert(d.type() == DateSet::Annual);
    }

    {
        d.set_month_set(singleSet, allMonths);
        assert(d.type() == DateSet::Monthly);
    }

    {
        d.set_month_set(singleSet, rangeSet);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1980));
        assert(d.type() == DateSet::Monthly);
    }

    {
        d.set_month_set(allMonthDays, allMonths);
        assert(d.type() == DateSet::Daily);
    }

    {
        d.set_month_set(allMonthDays, rangeSet);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1980));
        assert(d.type() == DateSet::Daily);
    }

    {
        d.set_month_set(rangeSet, singleSet);
        d.set_start(Date(1, Month::January(), 1980));
        d.set_finish(Date(31, Month::December(), 1980));
        assert(d.type() == DateSet::Daily);
    }
}
