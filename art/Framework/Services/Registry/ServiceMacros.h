#ifndef art_Framework_Services_Registry_ServiceMacros_h
#define art_Framework_Services_Registry_ServiceMacros_h

// ======================================================================
//
// ServiceMacros - Used to make an instance of a Service
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include "art/Utilities/TypeID.h"
#include "cpp0x/memory"

// ----------------------------------------------------------------------
#define DEFINE_ART_SERVICE_HOOK(klass) \
  extern "C" \
  art::TypeID \
  get_typeid() \
  { return art::TypeID(typeid(klass)); }

#define DEFINE_ART_SERVICE_INTERFACE_HOOK(klass) \
  extern "C" \
  art::TypeID \
  get_interface_typeid() \
  { return art::TypeID(typeid(klass)); }

#define DEFINE_ART_CONVERTER_HOOK(svc,iface) \
  extern "C" \
  std::unique_ptr<art::ServiceWrapperBase> \
  converter(std::shared_ptr<art::ServiceWrapperBase> const & swb) \
  { return std::unique_ptr<art::ServiceWrapperBase>( \
      static_cast<art::ServiceWrapperBase *>( \
        std::dynamic_pointer_cast<art::ServiceWrapper<svc>>(swb).get()->getAs<iface>() \
      ) \
    ); \
  }

#define DEFINE_ART_SYSTEM_SERVICE(svc) \
  DEFINE_ART_SERVICE_HOOK(svc)

#define DEFINE_ART_SYSTEM_SERVICE_INTERFACE_IMPL(svc,iface) \
  DEFINE_ART_SERVICE_HOOK(svc) \
  DEFINE_ART_SERVICE_INTERFACE_HOOK(iface) \
  DEFINE_ART_CONVERTER_HOOK(svc,iface)

#define DEFINE_ART_SERVICE(klass) \
  extern "C" \
  std::unique_ptr<art::ServiceWrapperBase> \
  make(fhicl::ParameterSet const & cfg, art::ActivityRegistry & reg) \
  { return std::unique_ptr<art::ServiceWrapperBase>( \
      new art::ServiceWrapper<klass>( \
        std::unique_ptr<klass>( \
          new klass(cfg,reg)) ) ); \
  } \
  DEFINE_ART_SERVICE_HOOK(klass)

#define DEFINE_ART_SERVICE_INTERFACE_IMPL(svc,iface) \
  DEFINE_ART_SERVICE(svc) \
  DEFINE_ART_SERVICE_INTERFACE_HOOK(iface) \
  DEFINE_ART_CONVERTER_HOOK(svc,iface)

// ======================================================================
#endif /* art_Framework_Services_Registry_ServiceMacros_h */

// Local Variables:
// mode: c++
// End:
