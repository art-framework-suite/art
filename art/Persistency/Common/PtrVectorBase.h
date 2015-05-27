#ifndef art_Persistency_Common_PtrVectorBase_h
#define art_Persistency_Common_PtrVectorBase_h

#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/RefCore.h"

#include <typeinfo>
#include <utility>
#include <vector>

class TBuffer;

namespace art {
  class PtrVectorBase;

  namespace detail {
    class PtrVectorBaseStreamer;
  }
}

class art::PtrVectorBase {
public:
  typedef unsigned long key_type;
protected:
  typedef std::vector<key_type> indices_t;
public:
  typedef indices_t::size_type size_type;

  virtual ~PtrVectorBase()
#ifndef __GCCXML__
  = default
#endif
    ;

  // Observers
  bool isNonnull() const;
  bool isNull() const;
  bool isAvailable() const;
  ProductID id() const;
  EDProductGetter const * productGetter() const;

  // Mutators
  void setProductGetter(EDProductGetter *g) const;

protected:
  PtrVectorBase()
#ifndef __GCCXML__
  = default
#endif
    ;

  void clear();
  void reserve(size_type n);
  void swap(PtrVectorBase &);
  void fillPtrs() const;
  void updateCore(RefCore const &core);

  template <typename T>
  typename Ptr<T>::key_type key(Ptr<T> const &ptr) const;

  bool operator==(PtrVectorBase const &) const;

private:
  RefCore core_;
  mutable indices_t indicies_; // Will be zeroed-out by fillPtrs();

  virtual void fill_offsets(indices_t &indices) = 0;
  virtual void fill_from_offsets(indices_t const &indices) const = 0;
  virtual void zeroTransients() = 0;

  friend class art::detail::PtrVectorBaseStreamer;
}; // PtrVectorBase

#ifndef __GCCXML__
inline bool
art::PtrVectorBase::isNonnull() const {
  return core_.isNonnull();
}

inline bool
art::PtrVectorBase::isNull() const {
  return ! isNonnull();
}

inline bool
art::PtrVectorBase::isAvailable() const {
  return core_.isAvailable();
}

inline art::ProductID
art::PtrVectorBase::id() const {
  return core_.id();
}

inline art::EDProductGetter const *
art::PtrVectorBase::productGetter() const {
  return core_.productGetter();
}

inline void
art::PtrVectorBase::setProductGetter(EDProductGetter *g) const {
  core_.setProductGetter(g);
}

inline void
art::PtrVectorBase::clear() {
  core_ = RefCore();
  indices_t tmp;
  indicies_.swap(tmp); // Free up memory
}

inline void
art::PtrVectorBase::reserve(size_type n) {
  indicies_.reserve(n);
}

inline void
art::PtrVectorBase::swap(PtrVectorBase &other) {
  core_.swap(other.core_);
}

inline void
art::PtrVectorBase::updateCore(RefCore const &core) {
  core_.pushBackItem(core);
}

template <typename T>
inline
typename art::Ptr<T>::key_type
art::PtrVectorBase::key(Ptr<T> const &ptr) const {
  return ptr.key();
}

inline
bool
art::PtrVectorBase::
operator==(PtrVectorBase const &other) const {
  return core_ == other.core_;
}

#endif /* __GCCXML__ */
#endif /* art_Persistency_Common_PtrVectorBase_h */

// Local Variables:
// mode: c++
// End:
