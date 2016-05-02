#ifndef art_Framework_Principal_Handle_h
#define art_Framework_Principal_Handle_h

// ======================================================================
//
// Handle: Non-owning "smart pointer" for reference to EDProducts and
//         their Provenances.
//
// ValidHandle: A Handle that can not be invalid, and thus does not check
//          for validity upon dereferencing.
//
// If the pointed-to EDProduct or Provenance is destroyed, use of the
// Handle becomes undefined. There is no way to query the Handle to
// discover if this has happened.
//
// Handles can have:
// -- Product and Provenance pointers both null;
// -- Both pointers valid
//
// ValidHandles must have Product and Provenance pointers valid.
//
// To check validity, one can use the Handle::isValid() function.
// ValidHandles cannot be invalid, and so have no validity checks.
//
// If failedToGet() returns true then the requested data is not available
// If failedToGet() returns false but isValid() is also false then no
// attempt to get data has occurred
//
// ======================================================================

#include "art/Framework/Principal/Group.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "art/Framework/Principal/Provenance.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/detail/metaprogramming.h"
#include "cetlib/demangle.h"
#include "cetlib/exception.h"

#include <memory>
#include <typeinfo>

namespace art {
  // defined herein:
  template <typename T> class Handle;
  template <typename T> class ValidHandle;
  template <class T> void swap(Handle<T> & a, Handle<T> & b);
  template <class T> void swap(ValidHandle<T> & a, ValidHandle<T> & b);
  template <class T> void convert_handle(GroupQueryResult const &, Handle<T> &);

  // forward declarations:
  class EDProduct;
  template <typename T>  class Wrapper;
}

// ======================================================================

template <typename T>
class art::Handle
{
public:
  typedef T element_type;
  class HandleTag { };

  // c'tors:
  Handle( );  // Default-constructed handles are invalid.
  Handle( GroupQueryResult const  & );

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  // pointer behaviors:
  T const &  operator * ( ) const;
  T const *  operator-> ( ) const; // alias for product()
  T const *  product    ( ) const;

  // inspectors:
  bool isValid( ) const;
  bool failedToGet( ) const; // was Handle used in a 'get' call whose data could not be found?
  Provenance const * provenance( ) const;
  ProductID id( ) const;
  std::shared_ptr<cet::exception const> whyFailed() const;

  // mutators:
  void swap( Handle<T> & other );
  void clear();

private:
  T const *                              prod_;
  Provenance                             prov_;
  std::shared_ptr<cet::exception const>  whyFailed_;

};  // Handle<>

// ----------------------------------------------------------------------
// c'tors:

template <class T>
art::Handle<T>::Handle() :
  prod_     (nullptr),
  prov_     (),
  whyFailed_()
{}

template <class T>
art::Handle<T>::Handle(GroupQueryResult const & gqr) :
  prod_     ( nullptr ),
  prov_     ( gqr.result() ),
  whyFailed_( gqr.whyFailed() )
{
  if (gqr.succeeded()) {
    auto const wrapperPtr =
      dynamic_cast<Wrapper<T> const *>(gqr.result()->
                                       uniqueProduct(TypeID(typeid(Wrapper<T>))));
    if (wrapperPtr == nullptr) {
      whyFailed_ = std::make_shared<art::Exception const>(errors::LogicError, "Handle<T> c'tor");
    } else {
      prod_ = wrapperPtr->product();
    }
  }
}

// ----------------------------------------------------------------------
// pointer behaviors:

template <class T>
inline
T const &
art::Handle<T>::operator *() const
{
  return *product();
}

template <class T>
T const *
art::Handle<T>::product() const
{
  if( failedToGet() )
    throw *whyFailed_;

  if( prod_ == nullptr )
    throw Exception(art::errors::NullPointerError)
      << "Attempt to de-reference product that points to 'nullptr'";

  return prod_;
}

template <class T>
inline
T const *
art::Handle<T>::operator->() const
{
  return product();
}

// ----------------------------------------------------------------------
// inspectors:

template <class T>
bool
art::Handle<T>::isValid() const
{
  return prod_ && prov_.isValid();
}

template <class T>
bool
art::Handle<T>::failedToGet() const
{
  return whyFailed_.get();
}

template <class T>
inline
art::Provenance const *
art::Handle<T>::provenance() const
{
  return & prov_;
}

template <class T>
inline
art::ProductID
art::Handle<T>::id() const
{
  return prov_.isValid() ? prov_.productID() : ProductID();
}

template <class T>
inline
std::shared_ptr<cet::exception const>
art::Handle<T>::whyFailed() const
{
  return whyFailed_;
}

// ----------------------------------------------------------------------
// mutators:

template <class T>
void
art::Handle<T>::swap(Handle<T> & other)
{
  using std::swap;
  swap(prod_, other.prod_);
  swap(prov_, other.prov_);
  swap(whyFailed_, other.whyFailed_);
}

template <class T>
void
art::Handle<T>::clear()
{
  prod_ = nullptr;
  prov_ = Provenance();
  whyFailed_.reset();
}

// ======================================================================
// Non-members:

template <class T>
inline
void
art::swap(Handle<T> & a, Handle<T> & b)
{
  a.swap(b);
}

// Convert from handle-to-EDProduct to handle-to-T
template <class T>
void
art::convert_handle(GroupQueryResult const & orig, Handle<T> & result)
{
  Handle<T> h(orig);
  swap(result, h);
}

// ======================================================================
template <typename T>
class art::ValidHandle
{
public:
  typedef T element_type;
  class HandleTag { };

  ValidHandle() = delete;
  ValidHandle(T const* prod, Provenance prov);
  ValidHandle(ValidHandle const&) = default;
  ValidHandle& operator=(ValidHandle const&) & = default;

  // pointer behaviors
  operator T const*() const;    // conversion to T const*
  T const & operator*() const;
  T const * operator->() const; // alias for product()
  T const * product() const;

  // inspectors
  bool isValid() const;  // always true
  bool failedToGet() const; // always false
  Provenance const* provenance() const;
  ProductID id() const;
  std::shared_ptr<cet::exception const> whyFailed() const; // always null

  // mutators
  void swap(ValidHandle<T>& other);
  // No clear() function, because a ValidHandle may not have an invalid
  // state, and that is what clear() would obtain.

private:
  T const*   prod_;
  Provenance prov_;
};

template <class T>
art::ValidHandle<T>::ValidHandle(T const* prod, Provenance prov) :
  prod_(prod),
  prov_(prov)
{
  if (prod == nullptr)
    throw Exception(art::errors::NullPointerError)
      << "Attempt to create ValidHandle with null pointer";
}

template <class T>
inline
art::ValidHandle<T>::operator T const* () const
{
  return prod_;
}

template <class T>
inline
T const &
art::ValidHandle<T>::operator*() const
{
  return *prod_;
}

template <class T>
inline
T const *
art::ValidHandle<T>::operator->() const
{
  return prod_;
}

template <class T>
inline
T const*
art::ValidHandle<T>::product() const
{
  return prod_;
}

template <class T>
inline
bool
art::ValidHandle<T>::isValid() const
{
  return true;
}

template <class T>
inline
bool
art::ValidHandle<T>::failedToGet() const
{
  return false;
}

template <class T>
inline
art::Provenance const *
art::ValidHandle<T>::provenance() const
{
  return & prov_;
}

template <class T>
inline
art::ProductID
art::ValidHandle<T>::id() const
{
  return prov_.productID();
}

template <class T>
inline
std::shared_ptr<cet::exception const>
art::ValidHandle<T>::whyFailed() const
{
  return std::shared_ptr<cet::exception const>();
}

template <class T>
inline
void
art::ValidHandle<T>::swap(art::ValidHandle<T>& other)
{
  std::swap(prod_, other.prod_);
  std::swap(prov_, other.prov_);
}

#endif /* art_Framework_Principal_Handle_h */

// Local Variables:
// mode: c++
// End:
