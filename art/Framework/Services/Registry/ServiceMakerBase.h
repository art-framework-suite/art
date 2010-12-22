#ifndef FWCore_ServiceRegistry_ServiceMakerBase_h
#define FWCore_ServiceRegistry_ServiceMakerBase_h

//
// ServiceMakerBase - Base class for Service Makers
//
// Internal detail of implementation of the ServiceRegistry system
//

#include "fhiclcpp/ParameterSet.h"

namespace art {
  class ActivityRegistry;

  namespace serviceregistry {

    class ServiceWrapperBase;
    class ServicesManager;

    class ServiceMakerBase {

    public:
      ServiceMakerBase();
      virtual ~ServiceMakerBase();

      // ---------- const member functions ---------------------
      virtual const std::type_info& serviceType() const = 0;

      virtual bool make(const fhicl::ParameterSet&,
                        art::ActivityRegistry&,
                        ServicesManager&) const = 0;

    private:
      // no copying
      ServiceMakerBase(const ServiceMakerBase&);
      const ServiceMakerBase& operator=(const ServiceMakerBase&);

    };

  }  // namespace serviceregistry
}  // namespace art

#endif  // FWCore_ServiceRegistry_ServiceMakerBase_h
