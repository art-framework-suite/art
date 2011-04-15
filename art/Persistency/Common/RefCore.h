#ifndef art_Persistency_Common_RefCore_h
#define art_Persistency_Common_RefCore_h

// ======================================================================
//
// RefCore: The component of art::Ref containing
//            - the product ID and
//            - the product getter.
//
// ======================================================================

#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Provenance/ProductID.h"
#include <algorithm>


namespace art {
  class EDProduct;
  class RefCore;

  bool  operator == ( RefCore const &, RefCore const & );
  bool  operator != ( RefCore const &, RefCore const & );
  bool  operator <  ( RefCore const &, RefCore const & );

  void  swap( art::RefCore &, art::RefCore & );
}

// ======================================================================

class art::RefCore
{
public:
  // --- Construction/destruction:

    RefCore()
  : id_        ( )
  , transients_( )
  { }

    RefCore( ProductID const &       theId
           , void const *            prodPtr
           , EDProductGetter const * prodGetter
           , bool                    transient )
  : id_        ( theId )
  , transients_( prodPtr, prodGetter, transient )
  { }

  // --- Observers:

  bool  isNonnull()   const { return isTransient() ? productPtr() != 0
                                                 : id_.isValid(); }
  bool  isNull()      const { return !isNonnull(); }
  bool  operator ! () const { return isNull(); }

  bool  isTransient()    const { return transients_.transient_; }
  int   isTransientInt() const { return transients_.transient_ ? 1 : 0; }
  // Checks if collection is in memory or available
  // in the Event. No type checking is done.
  bool  isAvailable()    const;

  ProductID         id()            const { return id_; }
  void const *      productPtr()    const { return transients_.itemPtr_; }
  EDProduct const * getProductPtr() const;

  // --- Mutators:

  void
    setProductPtr( void const * prodPtr ) const
  { transients_.setProductPtr(prodPtr); }

  EDProductGetter const * productGetter() const
  { return transients_.prodGetter_; }

  void
    setProductGetter( EDProductGetter const * prodGetter ) const;

  void
    swap( RefCore & other )
  { std::swap(id_, other.id_), std::swap(transients_, other.transients_); }

  void
    pushBackItem( RefCore const & productToBeInserted
                , bool            checkPointer );

  struct RefCoreTransients
  {
    // itemPtr_ is the address of the item for which the Ptr in which this
    // RefCoreTransients object resides is a pointer
    mutable void const *            itemPtr_;     // transient
    mutable EDProductGetter const * prodGetter_;  // transient
    bool                            transient_;   // transient

    // --- Construction/destruction:
      RefCoreTransients()
    : itemPtr_   ( 0 )
    , prodGetter_( 0 )
    , transient_ ( false )
    { }
    explicit
      RefCoreTransients( void const *            prodPtr
                       , EDProductGetter const * prodGetter
                       , bool                    transient )
    : itemPtr_   ( prodPtr )
    , prodGetter_( prodGetter )
    , transient_ ( transient )
    { }

    // --- Accessors:
    bool isTransient() const { return transient_; }

    // --- Mutators:
    void setProductGetter( EDProductGetter const * prodGetter ) const;
    void setProductPtr(void const * itemPtr) const { itemPtr_ = itemPtr; }

  };  // RefCoreTransients

private:

  void  setId( ProductID const & iId ) { id_ = iId; }
  void  setTransient() { transients_.transient_ = true; }

  ProductID          id_;
  RefCoreTransients  transients_;

};  // RefCore

// ======================================================================

inline bool
  art::operator == ( RefCore const & lhs, RefCore const & rhs )
{ return lhs.isTransient() == rhs.isTransient()
      && (lhs.isTransient() ? lhs.productPtr() == rhs.productPtr()
                            : lhs.id() == rhs.id()
         );
}

inline bool
  art::operator != ( RefCore const & lhs, RefCore const & rhs )
{ return ! (lhs == rhs); }

inline bool
  art::operator < ( RefCore const & lhs, RefCore const & rhs )
{ return lhs.isTransient()
       ? ( rhs.isTransient() ? lhs.productPtr() < rhs.productPtr() : false )
       : ( rhs.isTransient() ? true : lhs.id() < rhs.id() );
}

inline void
  art::swap( art::RefCore & lhs, art::RefCore & rhs )
{ lhs.swap(rhs); }

// ======================================================================

#endif /* art_Persistency_Common_RefCore_h */

// Local Variables:
// mode: c++
// End:
