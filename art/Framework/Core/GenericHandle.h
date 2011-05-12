#ifndef art_Framework_Core_GenericHandle_h
#define art_Framework_Core_GenericHandle_h
// -*- C++ -*-
//
// Package:     Framework
// Class  :     GenericHandle
//
/**\class GenericHandle GenericHandle.h FWCore/Framework/interface/GenericHandle.h

 Description: Allows interaction with data in the Event without actually using the C++ class

 Usage:
    The GenericHandle allows one to get data back from the art::Event as a Reflex::Object instead
  of as the actual C++ class type.

  //make a handle to hold an instance of 'MyClass'
  art::GenericHandle myHandle("MyClass");

  event.getByLabel("mine",myHandle);

  //call the print method of 'MyClass' instance
  myHandle->invoke("print);
*/

#include "Reflex/Object.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/FCPfwd.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Utilities/UseReflex.h"
#include "cpp0x/memory"
#include <string>

// forward declarations
namespace art {
   ///This class is just a 'tag' used to allow a specialization of art::Handle
struct GenericObject
{
};

template<>
class Handle<GenericObject> {
public:
      ///Throws exception if iName is not a known C++ class type
      Handle(std::string const& iName) :
        type_(Reflex::Type::ByName(iName)), prod_(), prov_(0), id_() {
           if(type_ == Reflex::Type()) {
              throw art::Exception(art::errors::NotFound)<<"Handle<GenericObject> told to use uknown type '"<<iName<<"'.\n Please check spelling or that a module uses this type in the job.";
           }
           if(type_.IsTypedef()){
              //For a 'Reflex::Typedef' the 'toType' method returns the actual type
              // this is needed since you are now allowed to 'invoke' methods of a 'Typedef'
              // only for a 'real' class
              type_ = type_.ToType();
           }
        }

   ///Throws exception if iType is invalid
   Handle(Reflex::Type const& iType):
      type_(iType), prod_(), prov_(0), id_() {
         if(iType == Reflex::Type()) {
            throw art::Exception(art::errors::NotFound)<<"Handle<GenericObject> given an invalid Reflex::Type";
         }
         if(type_.IsTypedef()){
            //For a 'Reflex::Typedef' the 'toType' method returns the actual type
            // this is needed since you are now allowed to 'invoke' methods of a 'Typedef'
            // only for a 'real' class
            type_ = type_.ToType();
         }
      }

   Handle(Handle<GenericObject> const& h):
   type_(h.type_),
   prod_(h.prod_),
   prov_(h.prov_),
   id_(h.id_),
   whyFailed_(h.whyFailed_)
   { }

   Handle(Reflex::Object const& prod, Provenance const* prov, ProductID const& pid):
   type_(prod.TypeOf()),
   prod_(prod),
   prov_(prov),
   id_(pid) {
      assert(prod_);
      assert(prov_);
      assert(id_ != ProductID());
   }

      //~Handle();

   void swap(Handle<GenericObject>& other)
   {
      // use unqualified swap for user defined classes
      using std::swap;
      swap(type_, other.type_);
      std::swap(prod_, other.prod_);
      swap(prov_, other.prov_);
      swap(id_, other.id_);
      swap(whyFailed_, other.whyFailed_);
   }


   Handle<GenericObject>& operator=(Handle<GenericObject> const& rhs)
   {
      Handle<GenericObject> temp(rhs);
      this->swap(temp);
      return *this;
   }

   bool isValid() const {
      return prod_ && 0!= prov_;
   }

   bool failedToGet() const {
     return 0 != whyFailed_.get();
   }
   Reflex::Object const* product() const {
     if(this->failedToGet()) {
       throw *whyFailed_;
     }
     return &prod_;
   }
   Reflex::Object const* operator->() const {return this->product();}
   Reflex::Object const& operator*() const {return *(this->product());}

   Reflex::Type const& type() const {return type_;}
   Provenance const* provenance() const {return prov_;}

   ProductID id() const {return id_;}

   void clear() { prov_ = 0; id_ = ProductID();
   whyFailed_.reset();}

   void setWhyFailed(std::shared_ptr<cet::exception> const& iWhyFailed) {
    whyFailed_=iWhyFailed;
  }
private:
   Reflex::Type type_;
   Reflex::Object prod_;
   Provenance const* prov_;
   ProductID id_;
   std::shared_ptr<cet::exception> whyFailed_;

};

typedef Handle<GenericObject> GenericHandle;

///specialize this function for GenericHandle
void convert_handle(BasicHandle const& orig,
                    Handle<GenericObject>& result);


///Specialize the Event's getByLabel method to work with a Handle<GenericObject>
template<>
bool
art::Event::getByLabel<GenericObject>(std::string const& label,
                                      std::string const& productInstanceName,
                                      Handle<GenericObject>& result) const;

template <>
bool
art::Event::getByLabel(art::InputTag const& tag, Handle<GenericObject>& result) const;

}
#endif /* art_Framework_Core_GenericHandle_h */

// Local Variables:
// mode: c++
// End:
