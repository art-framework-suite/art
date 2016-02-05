// ======================================================================
//
// EDProduct: The base class of each type that will be inserted into the
//            Event.
//
// ======================================================================

#include "canvas/Persistency/Common/EDProduct.h"

#include "canvas/Persistency/Provenance/ProductID.h"

using art::EDProduct;

void
EDProduct::setPtr(std::type_info const &toType,
                  unsigned long index,
                  void const * &ptr) const
{
   do_setPtr(toType, index, ptr);
}

void
EDProduct::getElementAddresses(std::type_info const &toType,
                         std::vector<unsigned long> const &indices,
                         std::vector<void const *> &ptr) const
{
   do_getElementAddresses(toType, indices, ptr);
}

// ======================================================================
