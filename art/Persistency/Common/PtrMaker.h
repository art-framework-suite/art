////////////////////////////////////////////////////////////////////////
// Class:       PtrMaker
// File:        art/art/Persistency/Common/PtrMaker.h
//
// Author:      Saba Sehrish
//
// Description: A common pattern is to create a collection A, and a
// collection B, create art::Ptrs to the objects in each of the
// collection and then create associations between the objects in the
// two collections. The purpose of art::PtrMaker is to simplify the
// process of creating art::Assns by providing a utility to create
// art::Ptrs. It is a two step process to create an art::Ptr with this
// approach.
//
// Step I has two cases; the product and ptrs are constructed in the
// same module, or in different modules. For each case, there is a way
// to construct a PtrMaker object.
//
// case 1: Construct a PtrMaker object that creates Ptrs into a
// collection of type C, the most common one is std::vector and hence
// is the default, created by the module of type Module, where the
// collection has instance name "instance", which is optional. For
// example, to create a PtrMaker for an std::vector<A> in an event
// evt, and current module, we will use the PtrMaker as follows:
//
//   PtrMaker<A> make_Aptr{evt, *this}; // or
//   auto make_Aptr = PtrMaker<A>::create<std::vector<A>>(evt, *this);
//
// If a container other std::vector<A> is desired, the static function
// 'create' must be used instead of one of the constructors.
//
// case 2: In this case, the collection of type C is created in
// another module. We need the product ID to create an object of
// PtrMaker. The way to get a product ID is to first get an
// art::Handle and then use "id()". Assuming, h is the art::Handle to
// the data product, and evt is art::Event, then we will use it as
// follows:
//
//   art::Handle<std::vector<A>> h;
//   PtrMaker<A> make_Aptr{evt, h.id()};
//
// Step II: Use an index to create an art::Ptr to an object in the
// slot indicated by "index"
//
//   auto const a = make_Aptr(index);
//
////////////////////////////////////////////////////////////////////////

#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/exception.h"

#include <iostream>
#include <string>
#include <vector>

namespace art {

  // To create art::Ptrs into a particular collection in an event,
  // subrun, run, or results.
  template <typename T>
  class PtrMaker {
  public:
    // Creates a PtrMaker that creates Ptrs into a collection of type
    // 'Container'.
    template <typename Container, typename DataLevel, typename Module>
    static PtrMaker<T> create(DataLevel const& E,
                              Module const& module,
                              std::string const& instance = {});

    // Creates a PtrMaker that creates Ptrs in to a collection of type
    // std::vector<T> created by the module of type Module, where the
    // collection has instance name "instance"
    template <typename DataLevel, typename Module>
    PtrMaker(DataLevel const& evt,
             Module const& module,
             std::string const& instance = {});

    // Use this constructor when making Ptrs to products created in
    // other modules
    template <typename DataLevel>
    PtrMaker(DataLevel const& evt, ProductID prodId);

    // Creates a Ptr to an object in the slot indicated by "index"
    Ptr<T> operator()(std::size_t index) const;

  private:
    ProductID const prodId_;
    EDProductGetter const* prodGetter_;
  };

  template <typename T>
  template <typename Container, typename DataLevel, typename Module>
  PtrMaker<T>
  PtrMaker<T>::create(DataLevel const& evt,
                      Module const& module,
                      std::string const& instance)
  {
    auto const pid = module.template getProductID<Container>(instance);
    return PtrMaker<T>{evt, pid};
  }

  template <typename T>
  template <typename DataLevel, typename Module>
  PtrMaker<T>::PtrMaker(DataLevel const& evt,
                        Module const& module,
                        std::string const& instance)
    : PtrMaker{evt, module.template getProductID<std::vector<T>>(instance)}
  {}

  template <typename T>
  template <typename DataLevel>
  PtrMaker<T>::PtrMaker(DataLevel const& evt, ProductID const pid)
    : prodId_{pid}, prodGetter_{evt.productGetter(pid)}
  {}

  template <typename T>
  Ptr<T>
  PtrMaker<T>::operator()(size_t const index) const
  {
    return Ptr<T>{prodId_, index, prodGetter_};
  }
}
// end of namespace
