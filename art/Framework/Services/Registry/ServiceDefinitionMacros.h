#ifndef art_Framework_Services_Registry_ServiceDefinitionMacros_h
#define art_Framework_Services_Registry_ServiceDefinitionMacros_h
////////////////////////////////////////////////////////////////////////
// ServiceDefinitionMacros - define the macros used to define an art
// service.
//
// Notes:
//
// * Unlike modules, services require macro invocations in their headers
//   *and* their compile units: see ServiceDeclarationMacros.h.
//
// * In the case where there is a user-callable interface (i.e. a public
//   header), the <service>_service.cc compilation unit should contain
//   *only* the appropriate definition macro and include the appropriate
//   header(s) to define the types used.
//
////////////////////////////////////////////////////////////////////////
// User-callable macros:
//
// DEFINE_ART_SERVICE(svc)
//   Define (in MyService_service.cc) the service declared with
//   DECLARE_ART_SERVICE.
//
// DEFINE_ART_SERVICE_INTERFACE_IMPL(svc, iface)
//   Define an art service of type <svc> implementing (inheriting from)
//   interface <iface>, with scope <scope>.
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Services/Registry/detail/definition_helper_macros.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

// User service macros, Service must provide a constructor with the
// correct signature (see notes above).

// Define a service.
#define DEFINE_ART_SERVICE(svc)                                                \
  DEFINE_ART_SH_CREATE(svc)                                                    \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(svc)

// Define a service implementing an interface.
#define DEFINE_ART_SERVICE_INTERFACE_IMPL(svc, iface)                          \
  DEFINE_ART_SERVICE(svc)                                                      \
  DEFINE_ART_SIH_CREATE(iface)

#endif /* art_Framework_Services_Registry_ServiceDefinitionMacros_h */

// Local Variables:
// mode: c++
// End:
