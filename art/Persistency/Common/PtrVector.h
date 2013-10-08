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

#include <vector>

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
  typedef typename data_t::const_iterator const_iterator;
  typedef typename data_t::iterator iterator;
  typedef typename data_t::const_reverse_iterator const_reverse_iterator;
  typedef typename data_t::reverse_iterator reverse_iterator;
  typedef typename data_t::value_type value_type;
  typedef typename data_t::const_reference const_reference;
  typedef typename data_t::reference reference;
  typedef typename data_t::size_type size_type;
  typedef typename data_t::allocator_type allocator_type;
  typedef typename data_t::pointer pointer;
  typedef typename data_t::const_pointer const_pointer;

  // Constructors
  PtrVector();
  template <typename U> PtrVector(PtrVector<U> const & other);

  // Observers
  bool operator==(PtrVector const & other) const;
  Ptr<T> const & operator[](unsigned long const idx) const;
  const_iterator begin() const;
  const_iterator end() const;
  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;
  bool empty() const;
  size_type size() const;
  size_type max_size() const;
  size_type capacity() const;
  const_reference at(size_type n) const;
  const_reference front() const;
  const_reference back() const;

  // Mutators
  template <typename U>
  // We can't ref-qualify assignment because of GCC_XML.
  PtrVector<T> & operator=(PtrVector<U> const & other);
  iterator begin();
  iterator end();
  reverse_iterator rbegin();
  reverse_iterator rend();
  template <typename U> void push_back(Ptr<U> const & p);
  void pop_back();

  template <typename U>
  iterator insert(iterator position, Ptr<U> const & p);
  template <typename U>
  void insert(iterator position, size_type n, Ptr<U> const & p);
  template <typename InputIterator>
  void insert(iterator position, InputIterator first, InputIterator last);
  template <typename U, typename InputIterator>
  void insert(iterator position, InputIterator first, InputIterator last);

  iterator erase(iterator position);
  iterator erase(iterator first, iterator last);
  void clear();
  void reserve(size_type n);
  reference at(size_type n);
  reference front();
  reference back();
  template <typename U>
  void assign(size_type n, Ptr<U> const & p);
  template <class InputIterator>
  void assign(InputIterator first, InputIterator last);
  template <typename U, class InputIterator>
  void assign(InputIterator first, InputIterator last);

  void swap(PtrVector & other);
  void swap(key_type k1, key_type k2);
  void sort();
  template <class COMP> void sort(COMP comp);

  static short Class_Version() { return 11; }

private:

  void fill_offsets(indices_t & indices);
  void fill_from_offsets(indices_t const & indices) const;
  void zeroTransients();

  // Need to explicitly zero this from custom streamer for base class.
  mutable data_t ptrs_; //! transient
}; // PtrVector<T>

#ifndef __GCCXML__
#include "cpp0x/functional"
#include "cpp0x/type_traits"

#include "boost/iterator.hpp"

#include <iterator>
template <typename T>
inline
art::PtrVector<T>::PtrVector()
  :
  PtrVectorBase()
{
}

template <typename T>
template <typename U>
inline
art::PtrVector<T>::PtrVector(PtrVector<U> const & other)
  :
  PtrVectorBase(other),
  ptrs_()
{
  // Ensure that types are compatible.
  STATIC_ASSERT((std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  ptrs_.reserve(other.size());
  std::copy(other.begin(), other.end(), std::back_inserter(ptrs_));
}

template <typename T>
inline bool
art::PtrVector<T>::operator==(PtrVector const & other) const
{
  return fillPtrs(),
         ptrs_ == other.ptrs_ &&
         this->PtrVectorBase::operator==(other);
}

template <typename T>
inline art::Ptr<T> const &
art::PtrVector<T>::operator[](unsigned long const idx) const
{
  return *(begin() + idx);
}

template <typename T>
inline typename art::PtrVector<T>::const_iterator
art::PtrVector<T>::begin() const
{
  return fillPtrs(), ptrs_.begin();
}

template <typename T>
inline typename art::PtrVector<T>::const_iterator
art::PtrVector<T>::end() const
{
  return fillPtrs(), ptrs_.end();
}

template <typename T>
inline typename art::PtrVector<T>::const_reverse_iterator
art::PtrVector<T>::rbegin() const
{
  return fillPtrs(), ptrs_.rbegin();
}

template <typename T>
inline typename art::PtrVector<T>::const_reverse_iterator
art::PtrVector<T>::rend() const
{
  return fillPtrs(), ptrs_.rend();
}

template <typename T>
inline
bool art::PtrVector<T>::empty() const
{
  return fillPtrs(), ptrs_.empty();
}

template <typename T>
inline typename art::PtrVector<T>::size_type
art::PtrVector<T>::size() const
{
  return fillPtrs(), ptrs_.size();
}

template <typename T>
inline typename art::PtrVector<T>::size_type
art::PtrVector<T>::max_size() const
{
  return fillPtrs(), ptrs_.max_size();
}

template <typename T>
inline typename art::PtrVector<T>::size_type
art::PtrVector<T>::capacity() const
{
  return ptrs_.capacity();
}

template <typename T>
inline typename art::PtrVector<T>::const_reference
art::PtrVector<T>::at(size_type n) const
{
  return fillPtrs(), ptrs_.at(n);
}

template <typename T>
inline typename art::PtrVector<T>::const_reference
art::PtrVector<T>::front() const
{
  return fillPtrs(), ptrs_.front();
}

template <typename T>
inline typename art::PtrVector<T>::const_reference
art::PtrVector<T>::back() const
{
  return fillPtrs(), ptrs_.back();
}

template <typename T>
template <typename U>
inline art::PtrVector<T> &
art::PtrVector<T>::operator=(PtrVector<U> const & other)
{
  STATIC_ASSERT((std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  assign<U>(other.begin(), other.end());
  this->PtrVectorBase::operator=(other);
}

template <typename T>
inline typename art::PtrVector<T>::iterator
art::PtrVector<T>::begin()
{
  return ptrs_.begin();
}

template <typename T>
inline typename art::PtrVector<T>::iterator
art::PtrVector<T>::end()
{
  return ptrs_.end();
}

template <typename T>
inline typename art::PtrVector<T>::reverse_iterator
art::PtrVector<T>::rbegin()
{
  return ptrs_.rbegin();
}

template <typename T>
inline typename art::PtrVector<T>::reverse_iterator
art::PtrVector<T>::rend()
{
  return ptrs_.rend();
}

template <typename T>
template <typename U>
inline void
art::PtrVector<T>::push_back(Ptr<U> const & p)
{
  // Ensure that types are compatible.
  STATIC_ASSERT((std::is_same<T, U>::value || std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  updateCore(p.refCore());
  ptrs_.push_back(p);
}

template <typename T>
inline void
art::PtrVector<T>::pop_back()
{
  ptrs_.pop_back();
}

template <typename T>
template <typename U>
inline typename art::PtrVector<T>::iterator
art::PtrVector<T>::insert(iterator position, Ptr<U> const & p)
{
  // Ensure that types are compatible.
  STATIC_ASSERT((std::is_same<T, U>::value || std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  updateCore(p);
  return ptrs_.insert(position, p);
}

template <typename T>
template <typename U>
inline void
art::PtrVector<T>::insert(iterator position, size_type n, Ptr<U> const & p)
{
  // Ensure that types are compatible.
  STATIC_ASSERT((std::is_same<T, U>::value || std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  updateCore(p);
  ptrs_.insert(position, n, p);
}

template <typename T>
template <typename InputIterator>
inline void
art::PtrVector<T>::insert(iterator position, InputIterator first, InputIterator last)
{
  using std::placeholders::_1;
  for_each(first, last, std::bind(&art::PtrVectorBase::updateCore, this, _1));
  ptrs_.insert(first, last);
}

template <typename T>
template <typename U, typename InputIterator>
inline void
art::PtrVector<T>::insert(iterator position, InputIterator first, InputIterator last)
{
  STATIC_ASSERT((std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  for (InputIterator i = first; i != last; ++i) {
    insert(position++, *i); // Not a simple forward to insert() due to type differences.
  }
}

template <typename T>
inline typename art::PtrVector<T>::iterator
art::PtrVector<T>::erase(iterator position)
{
  return ptrs_.erase(position);
}

template <typename T>
inline typename art::PtrVector<T>::iterator
art::PtrVector<T>::erase(iterator first, iterator last)
{
  return ptrs_.erase(first, last);
}

template <typename T>
inline void
art::PtrVector<T>::clear()
{
  ptrs_.clear();
  PtrVectorBase::clear();
}

template <typename T>
inline void
art::PtrVector<T>::reserve(size_type n)
{
  ptrs_.reserve(n);
}

template <typename T>
inline typename art::PtrVector<T>::reference
art::PtrVector<T>::at(size_type n)
{
  return fillPtrs(), ptrs_.at(n);
}

template <typename T>
inline typename art::PtrVector<T>::reference
art::PtrVector<T>::front()
{
  return fillPtrs(), ptrs_.front();
}

template <typename T>
inline typename art::PtrVector<T>::reference
art::PtrVector<T>::back()
{
  return fillPtrs(), ptrs_.back();
}

template <typename T>
template <typename U>
inline void
art::PtrVector<T>::assign(size_type n, Ptr<U> const & p)
{
  STATIC_ASSERT((std::is_same<T, U>::value || std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  PtrVectorBase::clear();
  updateCore(p);
  ptrs_.assign(n, p);
}

template <typename T>
template <typename InputIterator>
inline void
art::PtrVector<T>::assign(InputIterator first, InputIterator last)
{
  PtrVectorBase::clear();
  using std::placeholders::_1;
  for_each(first, last, std::bind(&art::PtrVectorBase::updateCore, this, _1));
  ptrs_.assign(first, last);
}

template <typename T>
template <typename U, typename InputIterator>
inline void
art::PtrVector<T>::assign(InputIterator first, InputIterator last)
{
  STATIC_ASSERT((std::is_base_of<T, U>::value || std::is_base_of<U, T>::value), "PtrVector: incompatible types");
  clear();
  for (InputIterator i = first; i != last; ++i) {
    push_back(*i); // Not a simple forward to assign() due to type differences.
  }
}

template <typename T>
inline void
art::PtrVector<T>::swap(PtrVector & other)
{
  ptrs_.swap(other.ptrs_);
  PtrVectorBase::swap(other);
}

template <typename T>
inline void
art::PtrVector<T>::swap(key_type k1, key_type k2)
{
  std::swap(ptrs_[k1], ptrs_[k2]);
}

template <typename T>
inline void
art::PtrVector<T>::sort()
{
  sort(std::less<T>());
}

template <typename T>
template <class COMP>
inline void
art::PtrVector<T>::sort(COMP comp)
{
  std::sort(ptrs_.begin(), ptrs_.end(), ComparePtrs<COMP>(comp));
}

template <typename T>
void
art::PtrVector<T>::fill_offsets(indices_t & indices)
{
  indices.reserve(ptrs_.size());
  for (typename data_t::const_iterator
       i = ptrs_.begin(),
       e = ptrs_.end();
       i != e;
       ++i) {
    indices.push_back(key(*i));
  }
}

template <typename T>
void
art::PtrVector<T>::fill_from_offsets(indices_t const & indices) const
{
  ptrs_.reserve(indices.size());
  for (typename indices_t::const_iterator
       i = indices.begin(),
       e = indices.end();
       i != e;
       ++i) {
    ptrs_.push_back(Ptr<T>(id(), *i, productGetter()));
  }
}

template <typename T>
inline void
art::PtrVector<T>::zeroTransients()
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
#endif /* art_Persistency_Common_PtrVector_h */

// Local Variables:
// mode: c++
// End:
