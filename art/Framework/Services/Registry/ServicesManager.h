#ifndef ServiceRegistry_ServicesManager_h
#define ServiceRegistry_ServicesManager_h

//
// Package:     ServiceRegistry
// Class  :     ServicesManager
//


#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceLegacy.h"
#include "art/Framework/Services/Registry/ServiceMakerBase.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Utilities/EDMException.h"
#include "art/Utilities/TypeIDBase.h"

#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <iostream>
#include <vector>


namespace edm {
  class ServiceToken;

  namespace serviceregistry {

    class ServicesManager
    {
    public:
      struct MakerHolder {
         MakerHolder(boost::shared_ptr<ServiceMakerBase> iMaker,
                     const fhicl::ParameterSet& iPSet,
                     edm::ActivityRegistry&);
         bool add(ServicesManager&) const;

         boost::shared_ptr<ServiceMakerBase> maker_;
         const fhicl::ParameterSet* pset_;
         edm::ActivityRegistry* registry_;
         mutable bool wasAdded_;
      };
      typedef std::map< TypeIDBase, boost::shared_ptr<ServiceWrapperBase> > Type2Service;
      typedef std::map< TypeIDBase, MakerHolder > Type2Maker;

      ServicesManager(const std::vector<fhicl::ParameterSet>& iConfiguration);

      /** Takes the services described by iToken and places them into the manager.
          Conflicts over Services provided by both the iToken and iConfiguration
          are resolved based on the value of iLegacy
      */
      ServicesManager(ServiceToken iToken,
                      ServiceLegacy iLegacy,
                      const std::vector<fhicl::ParameterSet>& iConfiguration);

      ~ServicesManager();

      // ---------- const member functions ---------------------
      template<class T>
	     T& get() const;

      ///returns true of the particular service is accessible
      template<class T>
         bool isAvailable() const {
            Type2Service::const_iterator itFound = type2Service_.find(TypeIDBase(typeid(T)));
            Type2Maker::const_iterator itFoundMaker;
            if(itFound == type2Service_.end()) {
               //do on demand building of the service
               if(0 == type2Maker_.get() ||
                   type2Maker_->end() == (itFoundMaker = type2Maker_->find(TypeIDBase(typeid(T))))) {
                  return false;
               } else {
                  //Actually create the service in order to 'flush out' any
                  // configuration errors for the service
                  itFoundMaker->second.add(const_cast<ServicesManager&>(*this));
                  itFound = type2Service_.find(TypeIDBase(typeid(T)));
                  //the 'add()' should have put the service into the list
                  assert(itFound != type2Service_.end());
               }
            }
            return true;
         }

      // ---------- member functions ---------------------------
      ///returns false if put fails because a service of this type already exists
      template<class T>
         bool put(boost::shared_ptr<ServiceWrapper<T> > iPtr) {
            Type2Service::const_iterator itFound = type2Service_.find(TypeIDBase(typeid(T)));
            if(itFound != type2Service_.end()) {
               return false;
            }
            type2Service_[ TypeIDBase(typeid(T)) ] = iPtr;
            actualCreationOrder_.push_back(TypeIDBase(typeid(T)));
            return true;
         }

      ///causes our ActivityRegistry's signals to be forwarded to iOther
      void connect(ActivityRegistry& iOther);

      ///causes iOther's signals to be forward to us
      void connectTo(ActivityRegistry& iOther);

      ///copy our Service's slots to the argument's signals
      void copySlotsTo(ActivityRegistry&);
      ///the copy the argument's slots to the our signals
      void copySlotsFrom(ActivityRegistry&);

    private:
      ServicesManager(const ServicesManager&); // stop default

      const ServicesManager& operator=(const ServicesManager&); // stop default

      void fillListOfMakers(const std::vector<fhicl::ParameterSet>&);
      void createServices();

      // ---------- member data --------------------------------
      //hold onto the Manager passed in from the ServiceToken so that
      // the ActivityRegistry of that Manager does not go out of scope
      // This must be first to get the Service destructors called in
      // the correct order.
      boost::shared_ptr<ServicesManager> associatedManager_;

      edm::ActivityRegistry registry_;
      Type2Service type2Service_;
      std::auto_ptr<Type2Maker> type2Maker_;
	 std::vector<TypeIDBase> requestedCreationOrder_;
	 std::vector<TypeIDBase> actualCreationOrder_;

    };  // ServicesManager

    template<class T>
      T& ServicesManager::get() const {
	  Type2Service::const_iterator itFound = type2Service_.find(TypeIDBase(typeid(T)));
	  Type2Maker::const_iterator itFoundMaker;
	  if(itFound == type2Service_.end()) {
	      //do on demand building of the service
	      if(0 == type2Maker_.get() ||
		 type2Maker_->end() == (itFoundMaker = type2Maker_->find(TypeIDBase(typeid(T))))) {
		  throw edm::Exception(edm::errors::NotFound,"Service Request")
                   <<" unable to find requested service with compiler type name '"<<typeid(T).name() <<"'.\n";
	      } else {
               itFoundMaker->second.add(const_cast<ServicesManager&>(*this));
               itFound = type2Service_.find(TypeIDBase(typeid(T)));
               //the 'add()' should have put the service into the list
               assert(itFound != type2Service_.end());
	      }
	  }
	  //convert it to its actual type
	  Type2Service::mapped_type second = itFound->second;

       boost::shared_ptr<ServiceWrapper<T> > tmpxx = boost::dynamic_pointer_cast<ServiceWrapper<T> >(second);
	  boost::shared_ptr<ServiceWrapper<T> > ptr(tmpxx);
	  assert(0 != ptr.get());
	  return ptr->get();
    }

  }  // namespace serviceregistry
}  // namespace edm

#endif  // ServiceRegistry_ServicesManager_h
