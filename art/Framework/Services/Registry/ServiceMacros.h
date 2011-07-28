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

#define DEFINE_ART_SYSTEM_SERVICE(klass) \
 DEFINE_ART_SERVICE_HOOK(klass)

#define DEFINE_ART_SERVICE(klass) \
extern "C" \
std::auto_ptr<art::ServiceWrapperBase> \
  make( fhicl::ParameterSet const & cfg, art::ActivityRegistry & reg ) \
{ return std::auto_ptr<art::ServiceWrapperBase>( \
    new art::ServiceWrapper<klass>( \
      std::auto_ptr<klass>( \
        new klass(cfg,reg)) ) ); \
} \
DEFINE_ART_SERVICE_HOOK(klass)

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceMacros_h */

// Local Variables:
// mode: c++
// End:
