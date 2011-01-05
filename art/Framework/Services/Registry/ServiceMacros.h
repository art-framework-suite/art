#ifndef ServiceRegistry_ServiceMacros_h
#define ServiceRegistry_ServiceMacros_h

// ======================================================================
//
// ServiceMacros - Used to make an instance of a Service
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include "art/Utilities/TypeIDBase.h"
#include <memory>

namespace art {
  class ActivityRegistry;
}
namespace fhicl {
  class ParameterSet;
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------

#define DEFINE_ART_SERVICE_HOOK(klass) \
extern "C" \
art::TypeIDBase \
  get_typeid() \
{ return art::TypeIDBase(typeid(klass)); }

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

#endif
