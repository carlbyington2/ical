/* Copyright (c) 1996  by Sanjay Ghemawat */
#ifndef _TABLEREP_H
#define _TABLEREP_H

// A "TableRep" is a tagged list of elements with the following interface.
#if 0
template <class K>
class TableRep {
  public:
    TableRep(int entries);
    ~TableRep();

    K& key(int i) const;

    // Optionally, a value may be stored as well.  If so, the following
    // method is added:
    V& val(int i) const;

    int  is_full(int i) const;
    int  is_empty(int i) const;
    int  is_del(int i) const;
    void make_full(int i, K);
    void make_empty(int i);
    void make_del(int i);
};
#endif

// This file defines a bunch of classes that conform to the
// "TableRep" signature.
//
// Array of tagged structures
//      StructTable1<K>
//      StructTable2<K,V>
//
// Split array for tags (to reduce space wastage for alignment
//      SplitTable1<K>
//      SplitTable1<K,V>

// An array of structures.
template <class K>
class StructTable1 {
  public:
    enum Tag { full, empty, del };
  private:
    struct Entry {
        Tag     tag;
        K       key;
    };
    Entry* table;
  public:
    StructTable1(int entries) {
        table = new Entry[entries];
        for (int i = entries-1; i >= 0; i--) {
            table[i].tag = empty;
        }
    }

    inline ~StructTable1() {
        delete [] table;
    }

    inline K& key(int i) const          { return table[i].key; }
    inline int is_full(int i) const     { return (table[i].tag == full); }
    inline int is_empty(int i) const    { return (table[i].tag == empty); }
    inline int is_del(int i) const      { return (table[i].tag == del); }
    inline void clear(int i)            { table[i].tag = empty; }
    inline void kill(int i)             { table[i].tag = del; }
    inline void store(int i, K k) {
        table[i].key = k;
        table[i].tag = full;
    }
};

// An array of structures.
template <class K, class V>
class StructTable2 {
  public:
    enum Tag { full, empty, del };
  private:
    struct Entry {
        Tag     tag;
        K       key;
        V       val;
    };
    Entry* table;
  public:
    StructTable2(int entries) {
        table = new Entry[entries];
        for (int i = entries-1; i >= 0; i--) {
            table[i].tag = empty;
        }
    }

    inline ~StructTable2() {
        delete [] table;
    }

    inline K& key(int i) const          { return table[i].key; }
    inline V& val(int i) const  { return table[i].val; }
    inline int is_full(int i) const     { return (table[i].tag == full); }
    inline int is_empty(int i) const    { return (table[i].tag == empty); }
    inline int is_del(int i) const      { return (table[i].tag == del); }
    inline void clear(int i)            { table[i].tag = empty; }
    inline void kill(int i)             { table[i].tag = del; }
    inline void store(int i, K k) {
        table[i].key = k;
        table[i].tag = full;
    }
};

// Split arrays of tags and values
template <class K>
class SplitTable1 {
  private:
    enum Tag { full, empty, del };
    char*       tag;            // List of tags
    K*          list;           // List of values
  public:
    SplitTable1(int entries) {
        list = new K[entries];
        tag  = new char[entries];
        for (int i = entries-1; i >= 0; i--) {
            tag[i] = empty;
        }
    }

    inline ~SplitTable1() {
        delete [] list;
        delete [] tag;
    }

    inline K& key(int i) const          { return list[i]; }
    inline int is_full(int i) const     { return (tag[i] == full); }
    inline int is_empty(int i) const    { return (tag[i] == empty); }
    inline int is_del(int i) const      { return (tag[i] == del); }
    inline void clear(int i)            { tag[i] = empty; }
    inline void kill(int i)             { tag[i] = del; }
    inline void store(int i, K k) {
        list[i] = k;
        tag[i] = full;
    }
};

// Split arrays of tags and values
template <class K, class V>
class SplitTable2 {
  private:
    enum Tag { full, empty, del };
    struct Entry {
        K       key;
        V       val;
    };
    char*       tag;            // List of tags
    Entry*      list;           // List of key,value pairs
  public:
    SplitTable2(int entries) {
        list = new Entry[entries];
        tag  = new char[entries];
        for (int i = entries-1; i >= 0; i--) {
            tag[i] = empty;
        }
    }

    inline ~SplitTable2() {
        delete [] list;
        delete [] tag;
    }

    inline K& key(int i) const          { return list[i].key; }
    inline V& val(int i) const          { return list[i].val; }
    inline int is_full(int i) const     { return (tag[i] == full); }
    inline int is_empty(int i) const    { return (tag[i] == empty); }
    inline int is_del(int i) const      { return (tag[i] == del); }
    inline void clear(int i)            { tag[i] = empty; }
    inline void kill(int i)             { tag[i] = del; }
    inline void store(int i, K k) {
        list[i].key = k;
        tag[i] = full;
    }
};

#endif /* _TABLEREP_H */
