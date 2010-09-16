//
// Package:     EDProduct
// Class  :     EDProductGetter
//


#include "art/Persistency/Common/EDProductGetter.h"

#include "art/Utilities/EDMException.h"

#include "boost/thread/tss.hpp"


namespace edm {

  //
  // constructors and destructor
  //
  EDProductGetter::EDProductGetter()
  {
  }

  EDProductGetter::~EDProductGetter()
  {
  }

  //
  // static member functions
  //

  namespace {
     struct Holder {
        Holder(): held_(0) {}
        edm::EDProductGetter const* held_;
     };
  }
  EDProductGetter const*
  EDProductGetter::set(EDProductGetter const* iGetter)
  {
     //NOTE: I use a Holder so that when s_registry goes away it will not delete the last EDProductGetter it saw
     static boost::thread_specific_ptr<Holder> s_registry;
     if(0 == s_registry.get()){
        s_registry.reset(new Holder);
     }
     EDProductGetter const* previous = s_registry->held_;
     s_registry->held_= iGetter;
     return previous;
  }

  EDProductGetter const*
  EDProductGetter::instance()
  {
     EDProductGetter const* returnValue = EDProductGetter::set(0);
     EDProductGetter::set(returnValue);
     return returnValue;
  }

  ProductID
  EDProductGetter::oldToNewProductID_(ProductID const&) const {
    throw edm::Exception(errors::LogicError)
        << "Internal error:  Call of oldToNewProductID_ for non-EventPrincipal.\n"
        << "Please report this error to the Framework group\n";
  }

  EDProductGetter const*
  mustBeNonZero(EDProductGetter const* prodGetter, std::string refType, ProductID const& productID) {
    if (prodGetter != 0) return prodGetter;
        throw Exception(errors::InvalidReference, refType)
        << "Attempt to construct a " << refType << " with ProductID " << productID << "\n"
        << "but with a null pointer to a product getter.\n"
        << "The product getter pointer passed to the constructor must refer\n"
        << "to a real getter, such as an EventPrincipal.\n";
  }

}  // namespace edm
