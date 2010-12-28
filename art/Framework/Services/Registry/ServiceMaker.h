#ifndef ServiceRegistry_ServiceMaker_h
#define ServiceRegistry_ServiceMaker_h

// ======================================================================
//
// ServiceMaker - Used to make an instance of a Service
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include "art/Utilities/TypeIDBase.h"
#include <memory>  // auto_ptr

namespace art {
  class ActivityRegistry;
}
namespace fhicl {
  class ParameterSet;
}

// ----------------------------------------------------------------------

extern "C"
{
  typedef art::TypeIDBase (*GET_TYPEID_t)();
  typedef std::auto_ptr<art::ServiceWrapperBase> (*MAKER_t)( fhicl::ParameterSet const &, art::ActivityRegistry & );
}

// ----------------------------------------------------------------------

#define DEFINE_ART_SERVICE(klass) \
extern "C" \
std::auto_ptr<art::ServiceWrapperBase> \
  make( fhicl::ParameterSet const & cfg, art::ActivityRegistry & reg ) \
{ return std::auto_ptr<art::ServiceWrapperBase>( \
    new art::ServiceWrapper<klass>( \
      std::auto_ptr<klass>( \
        new klass(cfg,reg))       )            ); \
} \
extern "C" \
art::TypeIDBase \
  get_typeid() \
{ return art::TypeIDBase(typeid(klass)); }

// ======================================================================

#endif  // ServiceRegistry_ServiceMaker_h
