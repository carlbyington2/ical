/* Copyright (c) 1995 by Sanjay Ghemawat */
#ifndef _ARRAYH
#define _ARRAYH

/*
 * Generic resizable array.
 *
 * Usage -
 *      declareArray(ArrayType,ElementType)
 *      implementArray(ArrayType,ElementType)
 *
 * For an array type without bounds checking ---
 *      declareUncheckedArray(ArrayType,ElementType)
 */

#include <assert.h>
#include "basic.h"

/*
 * Macro for generating array class.
 */

#define ArrayNoAssert(x) ((void)0)
#define declareArray(T,E) declareArrayType(T,E,assert)
#define declareUncheckedArray(T,E) declareArrayType(T,E,ArrayNoAssert)

#define declareArrayType(ArrayType,Element,ASSERT)                            \
class ArrayType {                                                             \
  public:                                                                     \
                                                                              \
    /* Constructors */                                                        \
    ArrayType();                        /* Empty array */                     \
    ArrayType(int predict);             /* Empty array with size predict */   \
    ArrayType(Element const*, int);     /* Initialized with C array */        \
    ArrayType(ArrayType const&);        /* Initialized with another Array */  \
    ArrayType(Element, int);            /* Fill with n copies of element */   \
                                                                              \
    /* Destructor */                                                          \
    ~ArrayType();                                                             \
                                                                              \
    /* Assignment operator */                                                 \
    ArrayType& operator=(ArrayType const&);                                   \
                                                                              \
    /* Array indexing */                                                      \
    Element& operator[](int index) const;                                     \
    Element& slot(int index) const;                                           \
                                                                              \
    /* Other Array operations */                                              \
    int  size() const;                  /* Return size; */                    \
    Element& high() const;              /* Return last element */             \
    Element* as_pointer() const;        /* Return as pointer to base */       \
    void append(Element v);             /* append an element */               \
    void append(Element, int n);        /* Append n copies of element */      \
    void concat(Element const*, int);   /* Concatenate C array */             \
    void concat(ArrayType const&);      /* Concatenate another Array */       \
    void fill(Element,int start,int n); /* Fill specified region with Elem */ \
    Element remove();                   /* Remove and return last element */  \
    void remove(int num);               /* Remove last num elements */        \
    void clear();                       /* Remove all elements */             \
                                                                              \
    /* Storage stuff */                                                       \
    void reclaim();                     /* Reclaim all unused space */        \
  private:                                                                    \
    Element*    store_;                 /* Actual storage */                  \
    int         alloc_;                 /* Size of allocated storage */       \
    int         size_;                  /* Size of used storage */            \
                                                                              \
    /* Storage enlargers */                                                   \
    void enlarge_allocation_to(int s);  /* Enlarge to s */                    \
    void enlarge_to(int s);             /* Enlarge to s if necessary */       \
                                                                              \
    /* Various checkers */                                                    \
    void bound_check(int) const;                                              \
    void range_check(int) const;                                              \
    void size_check(int) const;                                               \
};                                                                            \
                                                                              \
inline void ArrayType::bound_check(int val) const {                           \
    ASSERT(val >= 0);                                                         \
    ASSERT(val < size_);                                                      \
}                                                                             \
                                                                              \
inline void ArrayType::range_check(int val) const {                           \
    ASSERT(val >= 0);                                                         \
}                                                                             \
                                                                              \
inline void ArrayType::size_check(int val) const {                            \
    ASSERT(val >= 0);                                                         \
    ASSERT(val <= size_);                                                     \
}                                                                             \
                                                                              \
inline ArrayType::ArrayType() {                                               \
    alloc_ = 0;                                                               \
    size_  = 0;                                                               \
    store_ = 0;                                                               \
}                                                                             \
                                                                              \
inline int ArrayType::size() const {                                          \
    return size_;                                                             \
}                                                                             \
                                                                              \
inline Element& ArrayType::operator[](int index) const {                      \
    bound_check(index);                                                       \
    return store_[index];                                                     \
}                                                                             \
                                                                              \
inline Element& ArrayType::slot(int index) const {                            \
    bound_check(index);                                                       \
    return store_[index];                                                     \
}                                                                             \
                                                                              \
inline Element& ArrayType::high() const {                                     \
    bound_check(size_-1);                                                     \
    return store_[size_-1];                                                   \
}                                                                             \
                                                                              \
inline Element* ArrayType::as_pointer() const {                               \
    return store_;                                                            \
}                                                                             \
                                                                              \
inline void ArrayType::append(Element v) {                                    \
    if (size_ >= alloc_)                                                      \
        enlarge_allocation_to(size_+1);                                       \
    store_[size_++] = v;                                                      \
}                                                                             \
                                                                              \
inline Element ArrayType::remove() {                                          \
    if (size_ > 0) size_--;                                                   \
    return store_[size_];                                                     \
}                                                                             \
                                                                              \
inline void ArrayType::remove(int num) {                                      \
    size_check(num);                                                          \
    size_ -= num;                                                             \
}                                                                             \
                                                                              \
inline void ArrayType::clear() {                                              \
    size_ = 0;                                                                \
}                                                                             \

/*
 * Macro for non-inline array operations.
 */

#define implementArray(ArrayType,Element)                                     \
                                                                              \
ArrayType::ArrayType(int predict) {                                           \
    range_check(predict);                                                     \
    alloc_ = 0;                                                               \
    size_ = 0;                                                                \
    store_ = 0;                                                               \
    enlarge_to(predict);                                                      \
    size_ = 0;                                                                \
}                                                                             \
                                                                              \
ArrayType::~ArrayType() {                                                     \
    if (alloc_ > 0) delete [] store_;                                         \
}                                                                             \
                                                                              \
ArrayType::ArrayType(Element const* src, int s) {                             \
    range_check(s);                                                           \
    alloc_ = 0;                                                               \
    size_  = 0;                                                               \
    store_ = 0;                                                               \
    enlarge_to(s);                                                            \
    for (int i = 0; i < s; i++)                                               \
        store_[i] = src[i];                                                   \
}                                                                             \
                                                                              \
ArrayType::ArrayType(ArrayType const& d) {                                    \
    alloc_ = 0;                                                               \
    size_  = 0;                                                               \
    store_ = 0;                                                               \
    enlarge_to(d.size_);                                                      \
    for (int i = 0; i < size_; i++)                                           \
        store_[i] = d.store_[i];                                              \
}                                                                             \
                                                                              \
ArrayType::ArrayType(Element element, int numberOfCopies) {                   \
    range_check(numberOfCopies);                                              \
    alloc_ = 0;                                                               \
    size_ = 0;                                                                \
    store_ = 0;                                                               \
    enlarge_to(numberOfCopies);                                               \
    for (int i = 0; i < size_; i++)                                           \
        store_[i] = element;                                                  \
}                                                                             \
                                                                              \
ArrayType& ArrayType::operator=(ArrayType const& d) {                         \
    size_ = 0;                                                                \
    enlarge_to(d.size_);                                                      \
    for (int i = 0; i < size_; i++)                                           \
        store_[i] = d.store_[i];                                              \
    return (*this);                                                           \
}                                                                             \
                                                                              \
void ArrayType::append(Element element, int n) {                              \
    range_check(n);                                                           \
    int oldsize = size_;                                                      \
    enlarge_to(size_ + n);                                                    \
    for (int i = 0; i < n; i++)                                               \
        store_[i + oldsize] = element;                                        \
}                                                                             \
                                                                              \
void ArrayType::concat(ArrayType const& d) {                                  \
    int oldsize = size_;                                                      \
    enlarge_to(size_ + d.size_);                                              \
    for (int i = 0; i < d.size_; i++)                                         \
        store_[i+oldsize] = d.store_[i];                                      \
}                                                                             \
                                                                              \
void ArrayType::concat(Element const* src, int s) {                           \
    range_check(s);                                                           \
    int oldsize = size_;                                                      \
    enlarge_to(s + size_);                                                    \
    for (int i = 0; i < s; i++)                                               \
        store_[i+oldsize] = src[i];                                           \
}                                                                             \
                                                                              \
void ArrayType::fill(Element element, int start, int count) {                 \
    bound_check(start);                                                       \
    range_check(count);                                                       \
    bound_check(start + count - 1);                                           \
    for (int i = start; i < start + count; i++)                               \
        store_[i] = element;                                                  \
}                                                                             \
                                                                              \
void ArrayType::enlarge_to(int newsize) {                                     \
    if (newsize > alloc_)                                                     \
        enlarge_allocation_to(newsize);                                       \
   size_ = newsize;                                                           \
}                                                                             \
                                                                              \
void ArrayType::enlarge_allocation_to(int newsize) {                          \
    int newalloc = alloc_ * 2;                                                \
    if (newsize > newalloc) newalloc = newsize;                               \
                                                                              \
    Element* oldstore = store_;                                               \
    store_ = new Element[newalloc];                                           \
                                                                              \
    for (int i = 0; i < size_; i++)                                           \
        store_[i] = oldstore[i];                                              \
                                                                              \
    if (alloc_ > 0) delete [] oldstore;                                       \
    alloc_ = newalloc;                                                        \
}                                                                             \
                                                                              \
void ArrayType::reclaim() {                                                   \
    if (alloc_ > size_) {                                                     \
        /* Some free entries that can be reclaimed */                         \
        if (size_ > 0) {                                                      \
            /* Array not empty - create new store */                          \
            Element* newstore = new Element[size_];                           \
            for (int i = 0; i < size_; i++)                                   \
                newstore[i] = store_[i];                                      \
            delete [] store_;                                                 \
            alloc_ = size_;                                                   \
            store_ = newstore;                                                \
        }                                                                     \
        else {                                                                \
            /* Array empty - delete old store */                              \
            if (alloc_ > 0) {                                                 \
                delete [] store_;                                             \
                alloc_ = 0;                                                   \
            }                                                                 \
        }                                                                     \
    }                                                                         \
}                                                                             \

#endif /*_ARRAYH */
