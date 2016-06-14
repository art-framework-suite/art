#ifndef art_Persistency_Common_PtrVector_h
#define art_Persistency_Common_PtrVector_h

// ======================================================================
//
// PtrVector: a container which returns art::Ptr<>'s referring to items
// in one container in the art::Event
//
// ======================================================================

#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVectorBase.h"

#ifndef __GCCXML__
#include <initializer_list>
#include <iterator>
#endif
#include <vector>

#if GCC_IS_AT_LEAST(4,9,0)
#define PV_INSERT_POSITION_TYPE const_iterator
#else
#define PV_INSERT_POSITION_TYPE iterator
#endif

namespace art {
  template <typename T> class PtrVector;

  template <typename T>
  void swap(PtrVector<T> &, PtrVector<T> &);

  template <typename COMP>
  class ComparePtrs {
  public:
    ComparePtrs(COMP comp) : comp_(comp) {}
    template <typename T>
    bool operator()(Ptr<T> const & a,
                    Ptr<T> const & b) {
      return comp_(*a, *b);
    }
  private:
    COMP comp_;
  };
}

template <typename T>
class art::PtrVector : public PtrVectorBase {
private:
  typedef std::vector<Ptr<T> > data_t;
public:
  typedef typename data_t::value_type value_type;
  typedef typename data_t::allocator_type allocator_type;
  typedef typename data_t::reference reference;
  typedef typename data_t::const_reference const_reference;
  typedef typename data_t::pointer pointer;
  typedef typename data_t::const_pointer const_pointer;
  typedef typename data_t::iterator iterator;
  typedef typename data_t::const_iterator const_iterator;
  typedef typename data_t::reverse_iterator reverse_iterator;
  typedef typename data_t::const_reverse_iterator const_reverse_iterator;
  typedef typename data_t::difference_type difference_type;
  typedef typename data_t::size_type size_type;

  PtrVector();
  template <typename U> PtrVector(PtrVector<U> const & other);
#ifndef __GCCXML__
  template <typename U>
  PtrVector(std::initializer_list<Ptr<U> > il);
  template <typename U>
  PtrVector<T> & operator=(std::initializer_list<Ptr<U> > il);
#endif
  template <typename U> PtrVector<T> & operator=(PtrVector<U> const & other)
#ifndef __GCCXML__
 &
#endif
;

  // Iterators.
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
#ifndef __GCCXML__
  const_iterator cbegin() const; // C+2011.
  const_iterator cend() const; // C+2011.
  const_reverse_iterator crbegin() const; // C+2011.
  const_reverse_iterator crend() const; // C+2011.
#endif

  // Capacity.
  size_type size() const;
  size_type max_size() const;
  void resize(size_type n);
  size_type capacity() const;
  bool empty() const;
  void reserve(size_type n);
#ifndef __GCCXML__
  void shrink_to_fit(); // C+2011.
#endif

  // Element access.
  Ptr<T> const & operator[](unsigned long const idx) const;
  reference at(size_type n);
  const_reference at(size_type n) const;
  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;
  // No C++2011 data() functions by design.

  // Modifiers.
  template <typename U>
  void assign(size_type n, Ptr<U> const & p);
  template <class InputIterator>
  void assign(InputIterator first, InputIterator last);
#ifndef __GCCXML__
  template <typename U>
  void assign(std::initializer_list<Ptr<U> > il);
#endif
  template <typename U> void push_back(Ptr<U> const & p);
  void pop_back();
  template <typename U> iterator insert(iterator position, Ptr<U> const & p);
  template <typename U>
  void insert(iterator position, size_type n, Ptr<U> const & p);
  template <typename InputIterator>
  iterator insert(PV_INSERT_POSITION_TYPE position, InputIterator first, InputIterator last);
  iterator erase(iterator position);
  iterator erase(iterator first, iterator last);
  void swap(PtrVector & other);
  void swap(key_type k1, key_type k2);
  void clear();
  // No C++2011 emplace() due to problems associated with checking for
  // compatible ProductID.

  bool operator==(PtrVector const & other) const;
#ifndef __GCCXML__
  // Hide from ROOT to avoid enforced requirement of operator< on T.
  void sort();
  template <class COMP> void sort(COMP comp);
#endif
  static short Class_Version() { return 11; }

private:

  void fill_offsets(indices_t & indices)
#ifndef __GCCXML__
 override
#endif
;
  void fill_from_offsets(indices_t const & indices) const
#ifndef __GCCXML__
 override
#endif
;
  void zeroTransients()
#ifndef __GCCXML__
 override
#endif
;

  // Need to explicitly zero this from custom streamer for base class.
  mutable data_t ptrs_; //! transient
}; // PtrVector<T>

#ifndef __GCCXML__
#include <algorithm>
#include <functional>
#include <type_traits>
#include <iterator>
// #include "boost/iterator.hpp"

// Constructors.
template <typename T>
inline
art::PtrVector<T>::
PtrVector()
  :
  PtrVectorBase(),
  ptrs_()
{
}

template <typename T>
template <typename U>
inline
art::PtrVector<T>::
PtrVector(PtrVector<U> const & other)
  :
  PtrVectorBase(other),
  ptrs_()
{
  // Ensure that types are compatible.
  static_assert(std::is_base_of<T, U>::value || std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  ptrs_.reserve(other.size());
  std::copy(other.begin(), other.end(), std::back_inserter(ptrs_));
}

template <typename T>
template <typename U>
inline
art::PtrVector<T>::
PtrVector(std::initializer_list<Ptr<U> > il)
:
  PtrVectorBase(),
  ptrs_()
{
  static_assert(std::is_same<T, U>::value ||
                std::is_base_of<T, U>::value ||
                std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  ptrs_.reserve(il.size());
  for (auto && p : il) {
    updateCore(p.refCore());
    ptrs_.push_back(std::move(p));
  }
}

template <typename T>
template <typename U>
inline
art::PtrVector<T> &
art::PtrVector<T>::
operator=(std::initializer_list<Ptr<U> > il)
{
  static_assert(std::is_same<T, U>::value ||
                std::is_base_of<T, U>::value ||
                std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  assign(il);
  return *this;
}

template <typename T>
template <typename U>
inline
art::PtrVector<T> &
art::PtrVector<T>::
operator=(PtrVector<U> const & other) &
{
  static_assert(std::is_base_of<T, U>::value || std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  this->PtrVectorBase::operator=(other);
  ptrs_.clear();
  std::copy(other.cbegin(), other.cend(), std::back_inserter(ptrs_));
  return *this;
}

// Iterators.
template <typename T>
inline
auto
art::PtrVector<T>::
begin()
-> iterator
{
  return ptrs_.begin();
}

template <typename T>
inline
auto
art::PtrVector<T>::
begin() const
-> const_iterator
{
  return ptrs_.begin();
}

template <typename T>
inline
auto
art::PtrVector<T>::
end()
-> iterator
{
  return ptrs_.end();
}

template <typename T>
inline
auto
art::PtrVector<T>::
end() const
-> const_iterator
{
  return ptrs_.end();
}

template <typename T>
inline
auto
art::PtrVector<T>::
rbegin()
-> reverse_iterator
{
  return ptrs_.rbegin();
}

template <typename T>
inline
auto
art::PtrVector<T>::
rbegin() const
-> const_reverse_iterator
{
  return ptrs_.rbegin();
}

template <typename T>
inline
auto
art::PtrVector<T>::
rend()
-> reverse_iterator
{
  return ptrs_.rend();
}

template <typename T>
inline
auto
art::PtrVector<T>::
rend() const
-> const_reverse_iterator
{
  return ptrs_.rend();
}

template <typename T>
inline
auto
art::PtrVector<T>::
cbegin() const
-> const_iterator
{
  return ptrs_.cbegin();
}

template <typename T>
inline
auto
art::PtrVector<T>::
cend() const
-> const_iterator
{
  return ptrs_.cend();
}

template <typename T>
inline
auto
art::PtrVector<T>::
crbegin() const
-> const_reverse_iterator
{
  return ptrs_.crbegin();
}

template <typename T>
inline
auto
art::PtrVector<T>::
crend() const
-> const_reverse_iterator
{
  return ptrs_.crend();
}

// Capacity.
template <typename T>
inline
auto
art::PtrVector<T>::
size() const
->size_type
{
  return ptrs_.size();
}

template <typename T>
inline
auto
art::PtrVector<T>::
max_size() const
-> size_type
{
  return ptrs_.max_size();
}

template <typename T>
inline
void
art::PtrVector<T>::
resize(size_type n)
{
  ptrs_.resize(n);
}

template <typename T>
inline
auto
art::PtrVector<T>::
capacity() const
-> size_type
{
  return ptrs_.capacity();
}

template <typename T>
inline
bool
art::PtrVector<T>::
empty() const
{
  return ptrs_.empty();
}

template <typename T>
inline
void
art::PtrVector<T>::
reserve(size_type n)
{
  ptrs_.reserve(n);
}

template <typename T>
inline
void
art::PtrVector<T>::
shrink_to_fit()
{
  ptrs_.shrink_to_fit();
}

// Element access.
template <typename T>
inline
art::Ptr<T> const &
art::PtrVector<T>::
operator[](unsigned long const idx) const
{
  return *(begin() + idx);
}

template <typename T>
inline
auto
art::PtrVector<T>::
at(size_type n)
-> reference
{
  return ptrs_.at(n);
}

template <typename T>
inline
auto
art::PtrVector<T>::
at(size_type n) const
-> const_reference
{
  return ptrs_.at(n);
}

template <typename T>
inline
auto
art::PtrVector<T>::
front()
-> reference
{
  return ptrs_.front();
}

template <typename T>
inline
auto
art::PtrVector<T>::
front() const
-> const_reference
{
  return ptrs_.front();
}

template <typename T>
inline
auto
art::PtrVector<T>::
back()
-> reference
{
  return ptrs_.back();
}

template <typename T>
inline
auto
art::PtrVector<T>::
back() const
-> const_reference
{
  return ptrs_.back();
}

// Modifiers.
template <typename T>
template <typename U>
inline
void
art::PtrVector<T>::
assign(size_type n, Ptr<U> const & p)
{
  static_assert(std::is_same<T, U>::value ||
                std::is_base_of<T, U>::value ||
                std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  PtrVectorBase::clear();
  updateCore(p.refCore());
  ptrs_.assign(n, p);
}

template <typename T>
template <typename InputIterator>
inline
void
art::PtrVector<T>::
assign(InputIterator first, InputIterator last)
{
  PtrVectorBase::clear();
  std::for_each(first,
                last,
                [this](Ptr<T> const & p) { updateCore(p.refCore()); }
               );
  ptrs_.assign(first, last);
}

template <typename T>
template <typename U>
inline
void
art::PtrVector<T>::
assign(std::initializer_list<Ptr<U> > il)
{
  assign(il.begin(), il.end());
}

template <typename T>
template <typename U>
inline
void
art::PtrVector<T>::
push_back(Ptr<U> const & p)
{
  // Ensure that types are compatible.
  static_assert(std::is_same<T, U>::value ||
                std::is_base_of<T, U>::value ||
                std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  updateCore(p.refCore());
  ptrs_.push_back(p);
}

template <typename T>
inline
void
art::PtrVector<T>::
pop_back()
{
  ptrs_.pop_back();
}

template <typename T>
template <typename U>
inline typename art::PtrVector<T>::
iterator
art::PtrVector<T>::
insert(iterator position, Ptr<U> const & p)
{
  // Ensure that types are compatible.
  static_assert(std::is_same<T, U>::value ||
                std::is_base_of<T, U>::value ||
                std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  updateCore(p.refCore());
  return ptrs_.insert(position, p);
}

template <typename T>
template <typename U>
inline
void
art::PtrVector<T>::
insert(iterator position, size_type n, Ptr<U> const & p)
{
  // Ensure that types are compatible.
  static_assert(std::is_same<T, U>::value ||
                std::is_base_of<T, U>::value ||
                std::is_base_of<U, T>::value,
                "PtrVector: incompatible types");
  updateCore(p.refCore());
  ptrs_.insert(position, n, p);
}

template <typename T>
template <typename InputIterator>
inline
auto
art::PtrVector<T>::
insert(PV_INSERT_POSITION_TYPE position, InputIterator first, InputIterator last)
-> iterator
{
  std::for_each(first,
                last,
                [this](Ptr<T> const & p) { updateCore(p.refCore()); }
               );
#if GCC_IS_AT_LEAST(4,9,0)
  // C++2011.
  return ptrs_.insert(position, first, last);
#else
  // Inefficient with C++03 interface.
  auto const orig_dist = std::distance(ptrs_.begin(), position);
  ptrs_.insert(position, first, last);
  iterator result = ptrs_.begin();
  std::advance(result, orig_dist);
  return result;
#endif
}

template <typename T>
inline
auto
art::PtrVector<T>::
erase(iterator position)
-> iterator
{
  return ptrs_.erase(position);
}

template <typename T>
inline
auto
art::PtrVector<T>::
erase(iterator first, iterator last)
-> iterator
{
  return ptrs_.erase(first, last);
}

template <typename T>
inline
void
art::PtrVector<T>::
swap(PtrVector & other)
{
  ptrs_.swap(other.ptrs_);
  PtrVectorBase::swap(other);
}

template <typename T>
inline
void
art::PtrVector<T>::
swap(key_type k1, key_type k2)
{
  std::swap(ptrs_[k1], ptrs_[k2]);
}

template <typename T>
inline
void
art::PtrVector<T>::
clear()
{
  ptrs_.clear();
  PtrVectorBase::clear();
}

template <typename T>
inline
bool
art::PtrVector<T>::
operator==(PtrVector const & other) const
{
  return ptrs_ == other.ptrs_ &&
         this->PtrVectorBase::operator==(other);
}

template <typename T>
inline
void
art::PtrVector<T>::
sort()
{
  sort(std::less<T>());
}

template <typename T>
template <class COMP>
inline
void
art::PtrVector<T>::
sort(COMP comp)
{
  std::sort(ptrs_.begin(), ptrs_.end(), ComparePtrs<COMP>(comp));
}

template <typename T>
void
art::PtrVector<T>::
fill_offsets(indices_t & indices)
{
  // Precondition: indices is expected to be empty.
  assert(indices.empty());
  indices.reserve(ptrs_.size());
  for (auto const & i : ptrs_) {
    indices.push_back(i.key());
  }
}

template <typename T>
void
art::PtrVector<T>::
fill_from_offsets(indices_t const & indices) const
{
  // Precondition: ptrs_ is expected to be empty.
  assert(ptrs_.empty());
  ptrs_.reserve(indices.size());
  for (auto i : indices) {
    ptrs_.emplace_back(id(), i, productGetter());
  }
}

template <typename T>
inline
void
art::PtrVector<T>::
zeroTransients()
{
  data_t tmp;
  ptrs_.swap(tmp);
}

template <typename T>
inline void
art::swap(PtrVector<T> & lhs, PtrVector<T> & rhs)
{
  lhs.swap(rhs);
}

#endif // __GCCXML__

#undef PV_INSERT_POSITION_TYPE
#endif /* art_Persistency_Common_PtrVector_h */

// Local Variables:
// mode: c++
// End:
