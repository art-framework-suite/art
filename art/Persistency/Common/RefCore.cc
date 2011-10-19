// ======================================================================
//
// RefCore: The component of art::Ptr containing
//            - the product ID and
//            - the product getter.
//
// ======================================================================

#include "art/Persistency/Common/RefCore.h"

#include "art/Utilities/Exception.h"
#include <cassert>

using art::EDProduct;
using art::RefCore;

// ======================================================================

bool
  RefCore::isAvailable() const
{
    return productPtr() != 0
        || (    id_.isValid()
             && productGetter() != 0
////             && productGetter()->getIt(id_) != 0
             && productGetter()->getIt() != 0
           );
}  // RefCore::isAvailable()

EDProduct const *
  RefCore::getProductPtr() const
{
  // The following invariant would be nice to establish in all
  // constructors, but we can't be sure that the context in which
  // EDProductGetter::instance() is called will be one where a
  // non-null pointer is returned. The biggest question in the
  // various times at which Root makes RefCore instances, and how
  // old ones might be recycled.
  //
  // If our ProductID is non-null, we must have a way to get at the
  // product (unless it has been dropped). This means either we
  // already have the pointer to the product, or we have a valid
  // EDProductGetter to use.
  //
  //     assert(!id_.isValid() || productGetter() || itemPtr_);

  if( !id_.isValid()) {
    throw Exception(errors::InvalidReference, "BadRefCore")
      << "Detected an attempt to dereference a RefCore containing an\n"
         "invalid ProductID. Please modify the calling code to test\n"
         "validity before dereferencing.\n";
  }

  if( productPtr() == 0 && productGetter() == 0) {
    throw Exception(errors::InvalidReference, "BadRefCore")
      << "Detected an attempt to dereference a RefCore containing a\n"
         "valid ProductID, but neither a valid product pointer nor\n"
         "EDProductGetter has been detected. The calling code must be\n"
         "modified to establish a functioning EDProductGetter for the\n"
         "context in which this call is made.\n";
  }
////  return productGetter()->getIt(id_);
  return productGetter()->getIt();
}  // RefCore::getProductPtr()

void
  RefCore::setProductGetter( EDProductGetter const * prodGetter ) const
{ transients_.setProductGetter(prodGetter); }

void
  RefCore::pushBackItem( RefCore const & productToBeInserted
                       , bool            /*checkPointer*/ )
{
  if( productToBeInserted.isNull() ) {
    throw art::Exception(errors::InvalidReference, "Inconsistency")
      << "RefCore::pushBackItem: Ptr has invalid (zero) product ID,\n"
         "so it cannot be added to PtrVector. id should be ("
      << id() << ")\n";
  }

  if( isNull() ) {
    setId(productToBeInserted.id());
  }
  else if( id() != productToBeInserted.id() ) {
    throw art::Exception(errors::InvalidReference, "Inconsistency")
      << "RefCore::pushBackItem: Ptr is inconsistent with\n"
         "PtrVector. id = ("
      << productToBeInserted.id() << "), should be (" << id() << ")\n";
  }

  if(    productGetter() == 0
      && productToBeInserted.productGetter() != 0 )
    setProductGetter(productToBeInserted.productGetter());
  if(    productPtr() == 0
      && productToBeInserted.productPtr() != 0 )
    setProductPtr(productToBeInserted.productPtr());

}  // RefCore::pushBackItem()

// ======================================================================

void
  RefCore::RefCoreTransients
         ::setProductGetter( EDProductGetter const * prodGetter) const
{
  prodGetter_ = prodGetter;
}


// ======================================================================
