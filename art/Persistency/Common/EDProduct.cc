// ======================================================================
//
// EDProduct: The base class of each type that will be inserted into the
//            Event.
//
// ======================================================================

#include "art/Persistency/Common/EDProduct.h"

#include "art/Persistency/Provenance/ProductID.h"

using art::EDProduct;

EDProduct::EDProduct()
{ }

EDProduct::~EDProduct()
{ }

void
EDProduct::setPtr(std::type_info const &toType,
                  unsigned long index,
                  void const * &ptr) const
{
   do_setPtr(toType, index, ptr);
}

void
EDProduct::fillPtrVector(std::type_info const &toType,
                         std::vector<unsigned long> const &indices,
                         std::vector<void const *> &ptr) const
{
   do_fillPtrVector(toType, indices, ptr);
}

// ======================================================================
