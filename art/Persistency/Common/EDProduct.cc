// ======================================================================
//
// EDProduct: The base class of each type that will be inserted into the
//            Event.
//
// ======================================================================


#include "art/Persistency/Common/EDProduct.h"
using edm::EDProduct;


#include "art/Persistency/Provenance/ProductID.h"


  EDProduct::EDProduct()
{ }

  EDProduct::~EDProduct()
{ }

void
  EDProduct::setPtr( std::type_info const & iToType
                   , unsigned long          iIndex
                   , void const * &         oPtr ) const
{
  do_setPtr(iToType, iIndex, oPtr);
}

void
  EDProduct::fillPtrVector( std::type_info const &             iToType
                          , std::vector<unsigned long> const & iIndicies
                          , std::vector<void const *> &        oPtr) const
{
  do_fillPtrVector(iToType, iIndicies, oPtr);
}

// ======================================================================
