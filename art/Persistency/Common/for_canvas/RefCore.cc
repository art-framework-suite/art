// ======================================================================
//
// RefCore: The component of art::Ptr containing
//            - the product ID and
//            - the product getter.
//
// ======================================================================

#include "canvas/Persistency/Common/RefCore.h"

#include "canvas/Utilities/Exception.h"
#include <cassert>

bool
art::RefCore::isAvailable() const
{
  return productPtr() != 0 ||
    (id_.isValid() &&
     productGetter() != 0 &&
     productGetter()->isReady() &&
     productGetter()->getIt() != 0);
}

void
art::RefCore::pushBackItem(RefCore const & productToBeInserted)
{
  if (productToBeInserted.isNull()) {
    throw art::Exception(errors::InvalidReference, "Inconsistency")
        << "art::RefCore::pushBackItem: Ptr has invalid (zero) product ID,\n"
        "so it cannot be added to PtrVector. id should be ("
        << id() << ")\n";
  }
  if (isNull()) {
    id_ = productToBeInserted.id();
  }
  else if (id() != productToBeInserted.id()) {
    throw art::Exception(errors::InvalidReference, "Inconsistency")
      << "art::RefCore::pushBackItem: Ptr is inconsistent with\n"
      "PtrVector. id = ("
      << productToBeInserted.id()
      << "), should be ("
      << id()
      << ")\n";
  }
  if (productGetter() == 0
      && productToBeInserted.productGetter() != 0) {
    setProductGetter(productToBeInserted.productGetter());
  }
  if (productPtr() == 0 && productToBeInserted.productPtr() != 0) {
    setProductPtr(productToBeInserted.productPtr());
  }
}
