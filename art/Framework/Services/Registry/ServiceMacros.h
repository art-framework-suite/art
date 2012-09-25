#ifndef art_Framework_Services_Registry_ServiceMacros_h
#define art_Framework_Services_Registry_ServiceMacros_h

////////////////////////////////////////////////////////////////////////
//
// ServiceMacros - Used to make an instance of an art service.
//
// User-callable macros:
//
// DEFINE_ART_SERVICE(svc);
//   Define an art service of type <svc>.
//
// DEFINE_ART_SERVICE(svc, iface)
//   Define an art service of type <svc> implementing (inheriting from)
//   interface <iface>.
//
// Additionally, some services closely integrated with art may call:
//
// DEFINE_ART_SYSTEM_SERVICE(svc);
//
// and must be constructed by the art system rather than automatically
// as part of the service initialization process.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include "art/Utilities/TypeID.h"
#include "cpp0x/memory"

// Define a function returning the typeid of the service.
#define DEFINE_ART_SERVICE_TYPEID(svc) \
  extern "C" \
  art::TypeID \
  get_typeid() \
  { return art::TypeID(typeid(svc)); }

// Define a function returning the typeid of the interface.
#define DEFINE_ART_INTERFACE_TYPEID(iface) \
  extern "C" \
  art::TypeID \
  get_iface_typeid() \
  { return art::TypeID(typeid(iface)); }

// Define a function returning the typeid of the service implementing
// the interface.
#define DEFINE_ART_INTERFACE_IMPL_TYPEID(svc) \
  extern "C" \
  art::TypeID \
  get_iface_impl_typeid() \
  { return art::TypeID(typeid(svc)); }

// Define a function to convert a ServiceWrapper<svc> to a
// ServiceWrapper<iface>.
#define DEFINE_ART_CONVERTER(svc,iface) \
  extern "C" \
  std::unique_ptr<art::ServiceWrapperBase> \
  converter(std::shared_ptr<art::ServiceWrapperBase> const & swb) \
  { return std::unique_ptr<art::ServiceWrapperBase>( \
      static_cast<art::ServiceWrapperBase *>( \
        std::dynamic_pointer_cast<art::ServiceWrapper<svc>>(swb).get()->getAs<iface>() \
      ) \
    ); \
  }

// Define a system service (no maker).
#define DEFINE_ART_SYSTEM_SERVICE(svc) \
  DEFINE_ART_SERVICE_TYPEID(svc)

////////////////////////////////////////////////////////////////////////
// Note that it make very little sense to have a system service
// implementing an interface since it has to be constructed and added to
// the service cache list with no way to define the maker and converter
// functions. One could conceivably do a second add for the interface,
// but one would have to add interface to ServiceToken and
// ServicesManager (and ServicesManager::Cache). If we ever need it, it
// should be relatively trivial.
//
// 2012/09/24 CG.
////////////////////////////////////////////////////////////////////////

// Define a service.
#define DEFINE_ART_SERVICE(svc) \
  extern "C" \
  std::unique_ptr<art::ServiceWrapperBase> \
  make(fhicl::ParameterSet const & cfg, art::ActivityRegistry & reg) \
  { return std::unique_ptr<art::ServiceWrapperBase>( \
      new art::ServiceWrapper<svc>( \
        std::unique_ptr<svc>( \
          new svc(cfg,reg)) ) ); \
  } \
  DEFINE_ART_SERVICE_TYPEID(svc)

// Define a service implementing an interface.
#define DEFINE_ART_SERVICE_INTERFACE_IMPL(svc,iface) \
  DEFINE_ART_SERVICE(svc) \
  DEFINE_ART_INTERFACE_TYPEID(iface) \
  DEFINE_ART_INTERFACE_IMPL_TYPEID(svc) \
  DEFINE_ART_CONVERTER(svc,iface)

#endif /* art_Framework_Services_Registry_ServiceMacros_h */

// Local Variables:
// mode: c++
// End:
