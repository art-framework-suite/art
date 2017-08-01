////////////////////////////////////////////////////////////////////////
// Class:       PtrMaker
// File:        art/art/Persistency/Common/PtrMaker.h
// 
// Author:      Saba Sehrish
//
// Description: A common pattern is to create a collection A, 
// and a collection B, create art::Ptrs to the objects in each of 
// the collection and then create associations between the objects 
// in the two collections. The purpose of art::PtrMaker is to simplify 
// the process of creating art::Assns by providing a utility to create 
// art::Ptrs. It is a two step process to create an art::Ptr with this 
// approach. 
// Step I has two cases; the product and ptrs are constructed 
// in the same module, or in different modules. For each case, there 
// is a way to construct a PtrMaker object. 
//
// case 1: Construct a PtrMaker object that creates Ptrs in to a collection  
// of type C, the most common one is std::vector and hence is the default, 
// created by the module of type MODULETYPE, where the collection has 
// instance name "instance", which is optional. For example, to create 
// a PtrMaker for an std::vector of A in an event evt, and current module, 
// we will use the PtrMaker as follows:
// PtrMaker<A>make_Aptr(evt, *this);
// 
// case 2: In this case, the collection of type C is created in another 
// module. We need the product ID to create an object of PtrMaker. The
// way to get a product ID is to first get an art::Handle and then use 
// "id()". Assuming, h is the art::Handle to the data product, and evt is 
// art::Event, then we will use it as follows: 
// art::Handle<std::vector<A>> h;
// PtrMaker<A> make_Aptr(evt, h.id());
//
// Step II: Use an index to create an art::Ptr to an object in the 
// slot indicated by "index"
// auto const a = make_Aptr(index);
// 
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


#include <iostream>

namespace art {
   // to create art::Ptrs in to a particular collection in an event
   template <class T>
   class PtrMaker {
   public:
      //Creates a PtrMaker that creates Ptrs in to a collection of type C created by the module of type MODULETYPE, where the collection has instance name "instance"
      template <class MODULETYPE, class C = std::vector<T>>
      PtrMaker(art::Event const & evt, MODULETYPE const& module, std::string const& instance = std::string());
      
      //use this constructor when making Ptrs to products created in other modules
      PtrMaker(art::Event const & evt, const art::ProductID & prodId, std::string const& instance = std::string());
      
      //Creates a Ptr to an object in the slot indicated by "index"
      art::Ptr<T> operator()(std::size_t index) const;
      
   private:
      const art::ProductID prodId;
      art::EDProductGetter const* prodGetter;
   };
   
   template <class T>
   template <class MODULETYPE, class C>
   PtrMaker<T>::PtrMaker(art::Event const & evt, MODULETYPE const& module, std::string const & instance)
   : prodId(module.template getProductID<C>(evt, instance))
   , prodGetter(evt.productGetter(prodId))
   {  }
   
   template <class T>
   PtrMaker<T>::PtrMaker(art::Event const & evt, const art::ProductID & pid, std::string const & instance)
   : prodId(pid)
   , prodGetter(evt.productGetter(pid))
   {  }
   
   template <class T>
   art::Ptr<T> PtrMaker<T>::operator()(size_t index) const
   {
      art::Ptr<T> artPtr(prodId, index, prodGetter);
      return artPtr;
   }
}
// end of namespace

