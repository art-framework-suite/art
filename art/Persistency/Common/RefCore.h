#ifndef art_Persistency_Common_RefCore_h
#define art_Persistency_Common_RefCore_h

// ======================================================================
//
// RefCore: The component of art::Ptr containing
// - the product ID and
// - the product getter.
//
// ======================================================================

#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "cpp0x/algorithm"

namespace art {
  class EDProduct;
  class RefCore;

  bool operator == (RefCore const &, RefCore const &);
  bool operator != (RefCore const &, RefCore const &);
  bool operator < (RefCore const &, RefCore const &);

  void swap(art::RefCore &, art::RefCore &);
}

class art::RefCore {
public:
  RefCore();
  RefCore(ProductID const & theId,
          void const * prodPtr,
          EDProductGetter const * prodGetter);

  // Observers.
  bool isNonnull() const;
  bool isNull() const;
  bool operator !() const;

  // Checks if collection is in memory or available
  // in the Event; no type checking is done.
  bool isAvailable() const;

  ProductID id() const;
  void const * productPtr() const;
  EDProductGetter const * productGetter() const;

  // Modifiers.
  void setProductPtr(void const * prodPtr) const;
  void setProductGetter(EDProductGetter const * prodGetter) const;
  void swap(RefCore & other);
  void pushBackItem(RefCore const & productToBeInserted);

  struct RefCoreTransients {
    // itemPtr_ is the address of the item for which the Ptr in which
    // this RefCoreTransients object resides is a pointer.
    mutable void const * itemPtr_; // transient
    mutable EDProductGetter const * prodGetter_; // transient

    RefCoreTransients();
    explicit RefCoreTransients(void const * prodPtr,
                               EDProductGetter const * prodGetter);
  }; // RefCoreTransients

private:

  ProductID id_;
  RefCoreTransients transients_;
};

#ifndef __GCCXML__
inline
art::RefCore::
RefCore()
  :
  id_(),
  transients_()
{
}

inline
art::RefCore::
RefCore(ProductID const & id,
        void const * prodPtr,
        EDProductGetter const * prodGetter)
  :
  id_(id),
  transients_(prodPtr, prodGetter)
{
}

inline
bool
art::RefCore::
isNonnull() const
{
  return id_.isValid();
}

inline
bool
art::RefCore::
isNull() const
{
  return !isNonnull();
}

inline
bool
art::RefCore::
operator !() const
{
  return isNull();
}

inline
auto
art::RefCore::
id() const
-> ProductID
{
  return id_;
}

inline
void const *
art::RefCore::
productPtr() const
{
  return transients_.itemPtr_;
}

inline
auto
art::RefCore::
productGetter() const
-> EDProductGetter const *
{
  return transients_.prodGetter_;
}

inline
void
art::RefCore::
setProductPtr(void const * prodPtr) const
{
  transients_.itemPtr_ = prodPtr;
}

inline
void
art::RefCore::
setProductGetter(EDProductGetter const * prodGetter) const
{
  transients_.prodGetter_ = prodGetter;
}

inline
void
art::RefCore::
swap(RefCore & other)
{
  using std::swap;
  swap(id_, other.id_);
  swap(transients_, other.transients_);
}

inline
art::RefCore::RefCoreTransients::
RefCoreTransients()
:
  itemPtr_(0),
  prodGetter_(0)
{
}

inline
art::RefCore::RefCoreTransients::
RefCoreTransients(void const * prodPtr,
                  EDProductGetter const * prodGetter)
:
  itemPtr_(prodPtr),
  prodGetter_(prodGetter)
{
}

inline
bool
art::operator == (RefCore const & lhs, RefCore const & rhs)
{ return lhs.id() == rhs.id(); }

inline
bool
art::operator != (RefCore const & lhs, RefCore const & rhs)
{ return !(lhs == rhs); }

inline
bool
art::operator < (RefCore const & lhs, RefCore const & rhs)
{ return lhs.id() < rhs.id(); }

inline void
art::swap(art::RefCore & lhs, art::RefCore & rhs)
{ lhs.swap(rhs); }


#endif /* __GCCXML__ */
#endif /* art_Persistency_Common_RefCore_h */

// Local Variables:
// mode: c++
// End:
