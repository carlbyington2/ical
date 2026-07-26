/* Copyright (c) 1995 by Sanjay Ghemawat */
// This file is designed to be included multiple times and therefore
// no header guards should be placed on it.

// "Template" for generating various kinds of hash tables.
//
// This file should be included after defining the following macros
//
//      HTABLE          Table class name
//      HKEY            Key type
//      HASHER          One argument macro that hashes an HKEY to an int
//      HEQUAL          Optional equality operator (use == otherwise)
//      HVAL            Optional value type
//      HTABLEREP       Optional class conforming to "TableRep" specification
//
//      If HTABLE_IMPLEMENT is defined, then the implementation is
//      also emitted.  Otherwise, just the interface is emitted.

// Check that all of the macros are defined
#ifndef HTABLE
#error macro HTABLE not defined for hash table instantiation.
#endif
#ifndef HKEY
#error macro HKEY not defined for hash table instantiation.
#endif
#ifndef HASHER
#error macro HASHER not defined for hash table instantiation.
#endif
#ifndef HEQUAL
#define HEQUAL(a,b) ((a) == (b))
#endif

#ifndef HTABLEREP
#ifdef HVAL
#define HTABLEREP SplitTable2<HKEY,HVAL>
#else
#define HTABLEREP SplitTable1<HKEY>
#endif
#endif

#ifndef _TABLEREP_H
#include "tablerep.h"
#endif

// The cost-estimates for different operations are based on the
// assumption that the hash function is "good".

class HTABLE {
  public:
    HTABLE();
    HTABLE(HTABLE const&);
    ~HTABLE();

    HTABLE& operator = (HTABLE const&);

    int size() const;
    // effects  Returns number of entries in the table.
    //          This is an O(1) operation.

    int contains(HKEY x) const;
    // effects  Returns true iff the table contains the specified element.
    //          This is an O(1) operation.

#ifdef HVAL
    int find(HKEY key, HVAL& result) const;
    // modifies "result"
    // effects  If an element matching "key" exists in the table,
    //          store the corresponding value into "result" and return true.
    //          Else do not modify "result" and return false.
    //          This is an O(1) operation.
#endif

#ifdef HVAL
    int insert(HKEY key, HVAL v);
#else
    int insert(HKEY key);
#endif
    // modifies "this"
    // effects  If table contains an element matching "key", replace its
    //          associated value with "val" (if appropriate) and return false.
    //          Else insert "key/val" into the table and return true.
    //          This operation has amortized cost O(log n).
    //          Individual operations may cost upto O(n).
    //          If the size has been predicted correctly, the cost is O(1).

    int remove(HKEY x);
    // modifies "this"
    // effects  If table contains an element matching "x", remove that
    //          element and return true.  Else return false.
    //          This is an O(1) operation.

    void clear();
    // modifies "this"
    // effects  Removes all elements from "this".
    //          This is an O(n) operation.

    void check() const;
    // effects  Nothing.
    //          Checks the table for internal consistency.  Dies on error.

    void report_stats(char const* msg) const;
    // effects  Reports collected table statistics prefixed with "msg".
    //          Report is sent to stderr.

    void predict(int n);
    // effects  Resizes table to an appropriate size to hold "n" entries.
    //          This is an O(n) operation.

    void set_occupancy(int x);
    // effects  Set occupancy target to "x" percent.
    //          Values for "x" that are too small or too big to be
    //          beneficial are silently ignored.
    //          This is an O(n) operation.

    class Elements {
      public:
        Elements(HTABLE const* table);
        // requires  "table" is not modified while this generator is active.
        // effects   Generate each table element exactly once.
        //           Elements may be generated in arbitrary order.

        void operator = (HTABLE const* table);
        // modifies  "this"
        // effects   Initializes "this" to generate elements from "table".

#ifdef HVAL
        int get(HKEY& key, HVAL& val);
#else
        int get(HKEY& key);
#endif
        // modifies  "this", "key", "val".
        // effects    If more elements can be generated from the table,
        //            store the next entry in "key/val" and return true.
        //            Else return false.

        void del();
        // requires   Iterator has just yielded something
        // effects    Remove the last yielded element
      private:
        HTABLE const*   table;
        int             index;
    };

  private:
    friend class Elements;

    HTABLEREP*  table;                  // Tagged list of entries
    unsigned int  tsize;                // Size of the table (power of two)
    unsigned int  enlarge_size;         // Resize when count bigger or equal
    unsigned int  shrink_size;          // Resize when count bigger or equal
    unsigned int  count;                // Total # of full entries
    unsigned int  delcount;             // Number of deleted entries
    unsigned int  max_occupancy;        // Percent maximum occupancy in table

    // The following is maintained for fast arithmetic modulo "tsize"
    unsigned int  mask;                 // mask == tsize-1
    unsigned int  slot_bits;            // lg(tsize)

    void init(int s);
    // effects  ignores old state and sets new empty state to hold at
    //          least "s" elements.

    void resize(int n);
    // effects  Resizes the table to an appropriate size to hold "n" entries.

    unsigned int find_index(HKEY key) const;
    // effects  If element is already in the table, returns its index.
    //          Otherwise return an index suitable for inserting the element.
    //          The returned entry will be full if the element is already
    //          in the table, and empty or deleted otherwise.
    
    unsigned int table_size(unsigned int n) const;
    // effects  Return appropriate table size for "n" entries.

#ifdef HVAL
    void unsafe_insert(HKEY key, HVAL val, unsigned int hash_value);
#else
    void unsafe_insert(HKEY key, unsigned int hash_value);
#endif
    // requires "hash_value = hash_element(key)".
    //          "key" is not in the table.
    //          Table does not have any deleted entries.
    //          Table is large enough to have valid rep after
    //          "key" is inserted.
    //                  (i.e. "tsize > (count + 1)")
    // effects  Inserts "key/val" into the table.

    void check_enlarge();
    // effects  Check if table should be enlarged.
};

inline int HTABLE::size() const {
    return count;
}

inline void HTABLE::predict(int n) {
    resize(n);
}

inline HTABLE::Elements::Elements(HTABLE const* t) {
    table = t;
    index = t->tsize;
}

inline void HTABLE::Elements::operator = (HTABLE const* t) {
    table = t;
    index = t->tsize;
}

#ifdef HVAL
inline int HTABLE::Elements::get(HKEY& key, HVAL& val)
#else
inline int HTABLE::Elements::get(HKEY& key)
#endif
{
    while (1) {
        index--;
        if (index < 0) return 0;
        if (table->table->is_full(index)) {
            key = table->table->key(index);
#ifdef HVAL
            val = table->table->val(index);
#endif
            return 1;
        }
    }
}

#ifdef HTABLE_IMPLEMENT
#include "htable_p.h"
#endif

// Remove the controlling macros now
#undef HTABLE
#undef HKEY
#undef HVAL
#undef HASHER
#undef HEQUAL
#undef HTABLEREP
#undef HTABLE_IMPLEMENT
