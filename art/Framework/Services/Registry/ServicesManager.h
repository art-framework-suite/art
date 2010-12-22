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
#include "art/Framework/Core/LibraryManager.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeIDBase.h"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"
#include <cassert>
#include <iostream>
#include <vector>

/*
  We need a function in this thing that causes all the services to come to life when called.
 */


namespace art {
  class ServiceToken;
  
  namespace serviceregistry {
    
    class ServicesManager
    {
    public:
      
      using fhicl::ParameterSet;
      
      typedef std::vector<ParameterSet> ParameterSets;
      typedef boost::shared_ptr<ServiceWrapperBase> WrapperBase_ptr;
      
      struct Cache
      {
      Cache(ParameterSet const& pset, TypeIDBase id, MAKER maker):
	config_(pset), typeinfo_(id), make_(maker), service_() 
	{ }
	
      Cache(WrapperBase_ptr premade_service):
	config_(), typeinfo_(), make_(), service_(premade_service)
	{ }
	
	ParameterSet config_;
	TypeIDBase typeinfo_;
	MAKER make_;
	WrapperBase_ptr service_;
      };
      
      typedef std::map< TypeIDBase, Cache > Factory;
      typedef std::vector< TypeIDBase > TypeIDBases;
      typedef std::stack< WrapperBase_ptr > ServiceStack;

      /** Takes the services described by iToken and places them into the manager.
          Conflicts over Services provided by both the iToken and iConfiguration
          are resolved based on the value of iLegacy
      */

      ServicesManager(ParameterSets const& psets, art::LibraryManager const&);
      ServicesManager(ServiceToken iToken,ServiceLegacy iLegacy,
		      ParameterSets const& psets, art::LibraryManager const&);
      ~ServicesManager();

      // ---------- const member functions ---------------------
      template<class T> T& get() const;

      ///returns true of the particular service is accessible
      // what should this function really do?
      // NOTE: this function still needs to be rewritten.
      template<class T>
	bool isAvailable() const 
	{
	  Type2Service::const_iterator itFound = type2Service_.find(TypeIDBase(typeid(T)));
	  Type2Maker::const_iterator itFoundMaker;
	  
	  if(itFound == type2Service_.end()) 
	    {
	      //do on demand building of the service
	      if(0 == type2Maker_.get() ||
		 type2Maker_->end() == (itFoundMaker = type2Maker_->find(TypeIDBase(typeid(T))))) 
		return false;
		 
	      else
		{
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
      // NOTE: needs to be converted to returning a void.
      template<class T>
	bool put(boost::shared_ptr<ServiceWrapper<T> > premade_service) 
	{
	  TypeIDBase id(typeid(T));
	  Factory::const_iterator it = factory_.find(id);

	  if(it != factory_.end())
	    throw art::Exception(art::errors::LogicError,"Service")
	      << "The system has manually added service of type " << id.name()
	      << ", but the service system already has a configured service of that type\n";
        
	  factory_[ id ] = Cache(premade_service);
	  actualCreationOrder_.push(premade_service);
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
      void fillFactory(ParameterSets const&);

      // ---------- member data --------------------------------
      //hold onto the Manager passed in from the ServiceToken so that
      // the ActivityRegistry of that Manager does not go out of scope
      // This must be first to get the Service destructors called in
      // the correct order.

      // we will probably not be using this because we do not use the inheritance or sharing
      // features (although we may not understand this properly).
      boost::shared_ptr<ServicesManager> associatedManager_;
      
      // these are real things that we use.
      art::ActivityRegistry registry_;
      Factory factory_;

      // what the heck are these for? 
      TypeIDBases requestedCreationOrder_;
      ServiceStack actualCreationOrder_;
      
    };  // ServicesManager

    // NOTE: this function still needs to be rewritten.

    template<class T> T& ServicesManager::get() const
      {
	Type2Service::const_iterator itFound = type2Service_.find(TypeIDBase(typeid(T)));
	Type2Maker::const_iterator itFoundMaker;

	typedef ServiceWrapper<T> Wrapper;
	typedef boost::shared_ptr<Wrapper> Wrapper_ptr;

	if(itFound == type2Service_.end()) 
	  {
	    //do on demand building of the service
	    if(0 == type2Maker_.get() ||
	       type2Maker_->end() == (itFoundMaker = type2Maker_->find(TypeIDBase(typeid(T))))) 
	      {
		throw art::Exception(art::errors::NotFound,"Service Request")
		  <<" unable to find requested service with compiler type name '"<<typeid(T).name() <<"'.\n";
	      } 
	    else
	      {
		itFoundMaker->second.add(const_cast<ServicesManager&>(*this));
		itFound = type2Service_.find(TypeIDBase(typeid(T)));
		//the 'add()' should have put the service into the list
		assert(itFound != type2Service_.end());
	      }
	  }
	//convert it to its actual type
	Type2Service::mapped_type second = itFound->second;
	
	Wrapper_ptr tmpxx = boost::dynamic_pointer_cast<Wrapper>(second);
	Warpper_ptr ptr(tmpxx);

	assert(0 != ptr.get());
	return ptr->get();
      }

  }  // namespace serviceregistry
}  // namespace art

#endif  // ServiceRegistry_ServicesManager_h
