#ifndef art_Framework_Services_Registry_ServiceMacros_h
#define art_Framework_Services_Registry_ServiceMacros_h

////////////////////////////////////////////////////////////////////////
//
// ServiceMacros - define the macros used to declare and define an art
// service or an interface to be implemented by an art service.
//
// Notes:
//
// * Unlike modules, services require macro invocations in their
// headers *and* their compile units.
//
// * In order to declare your service, you must specify your service
// with a scope value of LEGACY, GLOBAL or PER_SCHEDULE, as follows:
//
//   * LEGACY services are unaware of multi-schedule operation, and
//     configuration of such a service precludes multi-schedule
//     operation. Such services must have a constructor:
//
//       MyService(fhicl::ParameterSet const&,
//                 art::ActivityRegistry&);
//
//   * GLOBAL services are expected to have the same signature as LEGACY
//     services, but warrant additionally that it is safe to call their
//     methods (including callbacks) from multiple schedules at the same
//     time. A global service may register for callbacks for per-
//     schedule signals (however the same callback must be registered
//     for all schedules per the ActivityRegistry interface).
//
//   * PER_SCHEDULE services must provide a signature:
//
//       MyService(fhicl::ParameterSet const&,
//                 art::ActivityRegistry&,
//                 art::ScheduleID);
//
//     Note that Per-schedule services may register for global callbacks,
//     but there will be n unique instances of the service (and
//     therefore unique callbacks), one for each schedule.
////////////////////////////////////////////////////////////////////////
//
// User-callable macros:
//
// DECLARE_ART_SERVICE(svc, scope)
//   Declare (in the header) an art service of type <svc> with scope
//   (see above).
//
// DEFINE_ART_SERVICE(svc)
//   Define (in MyService_service.cc) the service declared with
//   DECLARE_ART_SERVICE.
//
////////////////////////////////////
// DECLARE_ART_SERVICE_INTERFACE(iface, scope)
//   Declare an interface to be used by services (must be invoked in the
//   header for the interface, *not* any implementations thereof).
//
////////////////////////////////////
// DECLARE_ART_SERVICE_INTERFACE_IMPL(svc, iface, scope)
//   Declare (in the header) an art service of type <svc> implementing
//   (inheriting from) interface <iface>, with scope <scope>.
//
// DEFINE_ART_SERVICE_INTERFACE_IMPL(svc, iface)
//   Define an art service of type <svc> implementing (inheriting from)
//   interface <iface>, with scope <scope>.
//
////////////////////////////////////
// Some services closely integrated with art may call:
//
// DECLARE_ART_SYSTEM_SERVICE(svc, scope);
//
// and must be constructed by the art system rather than automatically
// as part of the service initialization process. A DEFINE... macro call
// is not necessary in this case.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/Framework/Services/Registry/detail/ServiceHandleAllowed.h"
#include "art/Framework/Services/Registry/detail/helper_macros.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

////////////////////////////////////////////////////////////////////////
// User service macros, Service must provide a constructor with the
// correct signature (see notes above).

////////////////////////////////////
// Multi-schedule-aware service declarations.

// Declare and define a service.
#define DECLARE_ART_SERVICE(svc, scope) DECLARE_ART_SERVICE_DETAIL(svc, scope)

#define DEFINE_ART_SERVICE(svc)                                                \
  DEFINE_ART_SH_CREATE(svc)                                                    \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(svc)

// Declare an interface.
#define DECLARE_ART_SERVICE_INTERFACE(svc, scope)                              \
  static_assert(                                                               \
    art::detail::handle_allowed_v<svc>,                                        \
    "\n\nart-error: You cannot create a service interface for type "           \
    "'" ART_DETAIL_STRINGIZED_TYPE(svc) "'.\n"                                 \
                                        "           There is a base class of " \
                                        "this type for which a ServiceHandle " \
                                        "cannot\n"                             \
                                        "           be constructed.  Please "  \
                                        "contact artists@fnal.gov for "        \
                                        "guidance.\n");                        \
  DECLARE_ART_SERVICE_INTERFACE_DETAIL(svc, scope)

// Declare and define a service implementing an interface.
#define DECLARE_ART_SERVICE_INTERFACE_IMPL(svc, iface, scope)                  \
  static_assert(                                                               \
    art::detail::handle_allowed_v<svc>,                                        \
    "\n\nart-error: You cannot create a service implementation for type "      \
    "'" ART_DETAIL_STRINGIZED_TYPE(svc) "'.\n"                                 \
                                        "           There is a base class of " \
                                        "this type for which a ServiceHandle " \
                                        "cannot\n"                             \
                                        "           be constructed.  Please "  \
                                        "contact artists@fnal.gov for "        \
                                        "guidance.\n");                        \
  DECLARE_ART_SERVICE_INTERFACE_IMPL_DETAIL(svc, iface, scope)

#define DEFINE_ART_SERVICE_INTERFACE_IMPL(svc, iface)                          \
  DEFINE_ART_SERVICE(svc)                                                      \
  DEFINE_ART_SIH_CREATE(iface)

// End of user service macros.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// No user-serviceable parts below.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// System service macros (requires support code in Event Processor)
// since they have no maker function.

#define DECLARE_ART_SYSTEM_SERVICE(svc, scope)                                 \
  DECLARE_ART_SYSTEM_SERVICE_DETAIL(svc, scope)

////////////////////////////////////////////////////////////////////////
// Note that it makes very little sense to have a system service
// implementing an interface since it has to be constructed and added to
// the service cache list with no way to define the maker and converter
// functions. One could conceivably do a second add for the interface,
// but one would have to add interface to ServiceToken and
// ServicesManager (and ServicesManager::Cache). If we ever need it, it
// should be relatively trivial.
//
// 2012/09/24 CG.
////////////////////////////////////////////////////////////////////////

#endif /* art_Framework_Services_Registry_ServiceMacros_h */

// Local Variables:
// mode: c++
// End:
