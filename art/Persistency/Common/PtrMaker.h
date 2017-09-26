#ifndef art_Persistency_Common_PtrMaker_h
#define art_Persistency_Common_PtrMaker_h
// vim: set sw=2 expandtab :
//
//  Class:       PtrMaker
//  File:        art/art/Persistency/Common/PtrMaker.h
//
//  Author:      Saba Sehrish
//
//  Description: A common pattern is to create a collection A,
//  and a collection B, create art::Ptrs to the objects in each of
//  the collection and then create associations between the objects
//  in the two collections. The purpose of art::PtrMaker is to simplify
//  the process of creating art::Assns by providing a utility to create
//  art::Ptrs. It is a two step process to create an art::Ptr with this
//  approach.
//  Step I has two cases; the product and ptrs are constructed
//  in the same module, or in different modules. For each case, there
//  is a way to construct a PtrMaker object.
//
//  case 1: Construct a PtrMaker object that creates Ptrs in to a collection
//  of type C, the most common one is std::vector and hence is the default,
//  created by the module of type MODULETYPE, where the collection has
//  instance name "instance", which is optional. For example, to create
//  a PtrMaker for an std::vector of A in an event evt, and current module,
//  we will use the PtrMaker as follows:
//  PtrMaker<A>make_Aptr(evt, *this);
//
//  case 2: In this case, the collection of type C is created in another
//  module. We need the product ID to create an object of PtrMaker. The
//  way to get a product ID is to first get an art::Handle and then use
//  "id()". Assuming, h is the art::Handle to the data product, and evt is
//  art::Event, then we will use it as follows:
//  art::Handle<std::vector<A>> h;
//  PtrMaker<A> make_Aptr(evt, h.id());
//
//  Step II: Use an index to create an art::Ptr to an object in the
//  slot indicated by "index"
//  auto const a = make_Aptr(index);
//

#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/ProductID.h"

#include <cstddef>
#include <string>
#include <vector>

namespace art {

class EDProductGetter;

// To create Ptrs in to a particular collection in an event
template <class T>
class PtrMaker {

public: // MEMBER FUNCTIONS -- Special Member Functions

  // Creates a PtrMaker that creates Ptrs in to a collection of type C
  // created by the module of type MODULETYPE, where the collection has
  // instance name "instance"
  template <class EVENT, class MODULETYPE, class C = std::vector<T>>
  PtrMaker(EVENT const& evt, MODULETYPE const& module,
           std::string const& instance = std::string());

  // use this constructor when making Ptrs to products created in other modules
  template <class EVENT>
  PtrMaker(EVENT const& evt, const ProductID& prodID);

public: // MEMBER FUNCTIONS -- API for the user

  // Creates a Ptr to an object in the slot indicated by "index"
  Ptr<T>
  operator()(std::size_t index) const;

private: // MEMBER DATA

  ProductID const
  prodID_;

  EDProductGetter const*
  prodGetter_;

};

template <class T>
template <class EVENT, class MODULETYPE, class C>
PtrMaker<T>::
PtrMaker(EVENT const& evt, MODULETYPE const& /*module*/, std::string const& instance)
  : prodID_{evt.template getProductID<C>(instance)}
  , prodGetter_{evt.productGetter(prodID_)}
{
}

template <class T>
template <class EVENT>
PtrMaker<T>::
PtrMaker(EVENT const& evt, const ProductID& pid)
  : prodID_(pid)
  , prodGetter_(evt.productGetter(pid))
{
}

template <class T>
Ptr<T>
PtrMaker<T>::
operator()(std::size_t index) const
{
  Ptr<T> artPtr(prodID_, index, prodGetter_);
  return artPtr;
}

} // namespace art

#endif /* art_Persistency_Common_PtrMaker_h */

// Local Variables:
// mode: c++
// End:
