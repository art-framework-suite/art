#ifndef art_Framework_Principal_Handle_h
#define art_Framework_Principal_Handle_h

// ======================================================================
//
// Handle: Non-owning "smart pointer" for reference to EDProducts and
//         their Provenances.
//
// If the pointed-to EDProduct or Provenance is destroyed, use of the
// Handle becomes undefined. There is no way to query the Handle to
// discover if this has happened.
//
// Handles can have:
// -- Product and Provenance pointers both null;
// -- Both pointers valid
//
// To check validity, one can use the isValid() function.
//
// If failedToGet() returns true then the requested data is not available
// If failedToGet() returns false but isValid() is also false then no
// attempt to get data has occurred
//
// ======================================================================

#include "art/Framework/Principal/Group.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Utilities/Exception.h"
#include "cetlib/demangle.h"
#include "cetlib/exception.h"
#include "cpp0x/memory"
#include <typeinfo>

namespace art {
  // defined herein:
  template <typename T>
    class Handle;
  template <class T>
    void  swap(Handle<T> & a, Handle<T> & b);
  template <class T>
    void  convert_handle(GroupQueryResult const &, Handle<T> &);

  // forward declarations:
  class EDProduct;
  template <typename T>  class Wrapper;
}

// ======================================================================

template <typename T>
class art::Handle
{
public:
  typedef  T  element_type;

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

  // mutators:
  void swap( Handle<T> & other );
  void clear( );

private:
  T const *                              prod_;
  Provenance                             prov_;
  std::shared_ptr<cet::exception const>  whyFailed_;

};  // Handle<>

// ----------------------------------------------------------------------
// c'tors:

template <class T>
art::Handle<T>::Handle( ) :
  prod_     ( 0 ),
  prov_     ( ),
  whyFailed_( )
{ }

template <class T>
art::Handle<T>::Handle(GroupQueryResult const & gqr) :
  prod_     ( 0 ),
  prov_     ( gqr.result() ),
  whyFailed_( gqr.whyFailed() )
{
  if( gqr.succeeded() )
    try {
      prod_ = dynamic_cast< Wrapper<T> const &>(*gqr.result()->product()
                                               ).product();
    }
    catch( std::bad_cast const & ) {
      typedef  cet::exception const  exc_t;
      whyFailed_ = std::shared_ptr<exc_t>( new art::Exception( errors::LogicError
                                                             , "Handle<T> c'tor"
                                         )                   );
    }
}

// ----------------------------------------------------------------------
// pointer behaviors:

template <class T>
inline
T const &
art::Handle<T>::operator *( ) const
{
  return *product();
}

template <class T>
T const *
art::Handle<T>::product( ) const
{
  if( failedToGet() ) {
    throw *whyFailed_;
  }
  // Should we throw if the pointer is null?
  return prod_;
}

template <class T>
inline
T const *
art::Handle<T>::operator->( ) const
{
  return product();
}

// ----------------------------------------------------------------------
// inspectors:

template <class T>
bool
art::Handle<T>::isValid( ) const
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
  // Should we throw if the pointer is null?
  return & prov_;
}

template <class T>
inline
art::ProductID
art::Handle<T>::id() const
{
  return prov_.isValid() ? prov_.productID() : ProductID();
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
  art::Handle<T>::clear( )
{
  prod_ = 0;
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

#endif /* art_Framework_Principal_Handle_h */

// Local Variables:
// mode: c++
// End:
