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
#include "art/Utilities/TypeID.h"
#include "cetlib/demangle.h"
#include "cpp0x/functional"
#include "cpp0x/type_traits"

#include "boost/iterator.hpp"

#include <iterator>
#include <vector>

#include "TClass.h"
#include "TROOT.h"

namespace art {
  template <typename> class PtrVector;

  template <typename T>
  void swap(PtrVector<T> &, PtrVector<T> &);

  template <typename COMP>
  class ComparePtrs {
  public:
    ComparePtrs(COMP comp) : comp_(comp) {}
    template <typename T>
    bool operator()(Ptr<T> const &a,
                    Ptr<T> const &b) {
      return comp_(*a, *b);
    }
  private:
    COMP comp_;
  };
}

template<typename T>
class art::PtrVector : public PtrVectorBase {
private:
  typedef std::vector<Ptr<T> > data_t;
public:
  typedef typename data_t::const_iterator const_iterator;
  typedef typename data_t::iterator iterator;
  typedef typename data_t::value_type value_type;
  typedef typename data_t::const_reference const_reference;
  typedef typename data_t::reference reference;
  typedef typename data_t::size_type size_type;

  // Constructors
  PtrVector();
  template<typename U> PtrVector(PtrVector<U> const & other);

  // Observers
  bool operator==(PtrVector const & other) const;
  Ptr<T> const &operator[](unsigned long const idx) const;
  const_iterator begin() const;
  const_iterator end() const;
  bool empty() const;
  size_type size() const;
  size_type capacity() const;

  // Mutators
  iterator begin();
  iterator end();
  void push_back(Ptr<T> const &p);
  template<typename U> void push_back(Ptr<U> const &p);
  void clear();
  void reserve(size_type n);
  void swap(PtrVector &other);
  void swap(key_type k1, key_type k2);
  void sort();
  template <class COMP> void sort(COMP comp);

  static short Class_Version() { return 10; }

private:

  static void setSplitting();

  void fill_offsets(indices_t &indices);
  void fill_from_offsets(indices_t const &indices) const;
  void zeroTransients();

  // Need to explicitly zero this from custom streamer for base class.
  mutable data_t ptrs_; //! transient
}; // PtrVector<T>

template <typename T>
inline
art::PtrVector<T>::PtrVector()
  :
  PtrVectorBase()
{
  setSplitting(); // FIXME: not thread-safe.
}

template <typename T>
template <typename U>
inline
art::PtrVector<T>::PtrVector(PtrVector<U> const &other)
  :
  PtrVectorBase(other)
{
  // Ensure that types are compatible.
  STATIC_ASSERT(( std::is_base_of<T,U>::value ), "PtrVector: incompatible types");
}

template <typename T>
inline bool
art::PtrVector<T>::operator==(PtrVector const &other) const {
  return fillPtrs(),
    ptrs_ == other.ptrs_ &&
    this->PtrVectorBase::operator==(other);
}

template <typename T>
inline art::Ptr<T> const &
art::PtrVector<T>::operator[](unsigned long const idx) const {
  return *(begin() + idx);
}

template <typename T>
inline typename art::PtrVector<T>::const_iterator
art::PtrVector<T>::begin() const {
  return fillPtrs(), ptrs_.begin();
}

template <typename T>
inline typename art::PtrVector<T>::const_iterator
art::PtrVector<T>::end() const {
  return fillPtrs(), ptrs_.end();
}

template <typename T>
inline
bool art::PtrVector<T>::empty() const {
  return fillPtrs(), ptrs_.empty();
}

template <typename T>
inline typename art::PtrVector<T>::size_type
art::PtrVector<T>::size() const {
  return fillPtrs(), ptrs_.size();
}

template <typename T>
inline typename art::PtrVector<T>::size_type
art::PtrVector<T>::capacity() const {
  return ptrs_.capacity();
}

template <typename T>
inline typename art::PtrVector<T>::iterator
art::PtrVector<T>::begin() {
  return ptrs_.begin();
}

template <typename T>
inline typename art::PtrVector<T>::iterator
art::PtrVector<T>::end() {
  return ptrs_.end();
}

template <typename T>
inline void
art::PtrVector<T>::push_back(Ptr<T> const &p) {
  updateCore(p.refCore());
  ptrs_.push_back(p);
}

template<typename T>
template<typename U>
inline void
art::PtrVector<T>::push_back(Ptr<U> const &p) {
  // Ensure that types are compatible.
  STATIC_ASSERT(( std::is_base_of<T,U>::value ), "PtrVector: incompatible types");
  updateCore(p.refCore());
  ptrs_.push_back(p);
}

template <typename T>
inline void
art::PtrVector<T>::clear() {
  ptrs_.clear();
  PtrVectorBase::clear();
}

template <typename T>
inline void
art::PtrVector<T>::reserve(size_type n) {
  ptrs_.reserve(n);
}

template <typename T>
inline void
art::PtrVector<T>::swap(PtrVector &other) {
  ptrs_.swap(other.ptrs_);
  PtrVectorBase::swap(other);
}

template <typename T>
inline void
art::PtrVector<T>::swap(key_type k1, key_type k2) {
  std::swap(ptrs_[k1], ptrs_[k2]);
}

template <typename T>
inline void
art::PtrVector<T>::sort() {
  sort(std::less<T>());
}

template <typename T> 
template <class COMP>
inline void
art::PtrVector<T>::sort(COMP comp) {
  std::sort(ptrs_.begin(), ptrs_.end(), ComparePtrs<COMP>(comp));
}

template <typename T>
void
art::PtrVector<T>::setSplitting() {
  static bool firstCall = true;
  if (firstCall) {
    TClass *cl = gROOT->GetClass(typeid(PtrVector<T>));
    if (!cl) {
      throw Exception(errors::DictionaryNotFound)
        << "art::PtrVector::setSplitting(): No dictionary for class "
        << cet::demangle_symbol(typeid(PtrVector<T>).name())
        << "\n";
    }
    cl->SetCanSplit(0);
    firstCall = false;
  }
}

template <typename T>
void
art::PtrVector<T>::fill_offsets(indices_t &indices) {
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
art::PtrVector<T>::fill_from_offsets(indices_t const &indices) const {
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
art::PtrVector<T>::zeroTransients() {
  data_t tmp;
  ptrs_.swap(tmp);
}

template<typename T>
inline void
art::swap(PtrVector<T> &lhs, PtrVector<T> &rhs) {
  lhs.swap(rhs);
}

#endif /* art_Persistency_Common_PtrVector_h */

// Local Variables:
// mode: c++
// End:
