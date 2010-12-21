#ifndef ServiceRegistry_ServiceWrapper_h
#define ServiceRegistry_ServiceWrapper_h

//
// Package:     ServiceRegistry
// Class  :     ServiceWrapper
//
/**\class ServiceWrapper ServiceWrapper.h FWCore/ServiceRegistry/interface/ServiceWrapper.h

 Description: Class template through which the framework manages the lifetime of a service.

 Usage:
    Implementation detail of the ServiceRegistry system

*/


#include "art/Framework/Services/Registry/ServiceWrapperBase.h"

#include "fhiclcpp/ParameterSet.h"

#include <memory>


namespace art {
  class ActivityRegistry;

  namespace serviceregistry {

    template< class T >
    class ServiceWrapper : public ServiceWrapperBase
    {
    public:
      ServiceWrapper(std::auto_ptr<T> iService) :
      service_(iService) {}

      // ---------- const member functions ---------------------
      T& get() const { return *service_; }

    private:
      // no copying
      ServiceWrapper(const ServiceWrapper&);
      const ServiceWrapper& operator=(const ServiceWrapper&);

      // ---------- member data --------------------------------
      std::auto_ptr<T> service_;

    };

  }  // namespace serviceregistry
}  // namespace art

#endif  // ServiceRegistry_ServiceWrapper_h
