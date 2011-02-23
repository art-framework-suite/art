// ======================================================================
//
// PtrVectorBase: provide PtrVector<T> behavior that's independent of T
//
// ======================================================================

// Class definition:
#include "art/Persistency/Common/PtrVectorBase.h"
using art::PtrVectorBase;

// Framework support:
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/traits.h"
#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"

// C++ support:
#include <algorithm>


// Constructor and destructor:

  PtrVectorBase::PtrVectorBase()
: core_       ( )
, indicies_   ( )
, cachedItems_( )
{ }

  PtrVectorBase::~PtrVectorBase()
{ }

// Mutators:

void
  PtrVectorBase::push_back_base( RefCore const & core
                               , key_type        key
                               , void const *    data )
{
  core_.pushBackItem(core, false);

  // Did we already push a 'non-cached' Ptr into the container,
  // or is this a 'non-cached' Ptr?
  if( indicies_.size() == cachedItems_.size() ) {
    if( data )                                    cachedItems_.push_back(data);
    else if( key_traits<key_type>::value == key ) cachedItems_.push_back(0);
    else                                          cachedItems_.clear();
  }
  indicies_.push_back(key);
}

void
  PtrVectorBase::swap( PtrVectorBase & other )
{
  core_       .swap(other.core_);
  indicies_   .swap(other.indicies_);
  cachedItems_.swap(other.cachedItems_);
}

// --- Helpers:

void
  PtrVectorBase::getProduct_() const
{
  if( hasCache() )
    return;
  if( indicies_.size() == 0 )
    return;
  if( 0 == productGetter())
    throw art::Exception(art::errors::LogicError)
          << "Tried to get data for a PtrVector which has no EDProductGetter\n";

  EDProduct const * product = productGetter()->getIt(id());

  if( 0 == product )
    throw art::Exception(art::errors::InvalidReference)
          << "Asked for data from a PtrVector"
             " that refers to a non-existent product with id "
          << id()
          << '\n';

  product->getElementAddresses(typeInfo(), indicies_, cachedItems_);
}

bool
  PtrVectorBase::operator == ( PtrVectorBase const & other ) const
{
  return core_ == other.core_ && indicies_.size() == other.indicies_.size()
       ? std::equal( indicies_.begin(), indicies_.end()
                   , other.indicies_.begin() )
       : false;

}
