#ifndef art_Framework_Services_Registry_ServiceDeclarationMacros_h
#define art_Framework_Services_Registry_ServiceDeclarationMacros_h

//
// ServiceDeclarationMacros - define the macros used to declare an art
// service or an interface to be implemented by an art service.
//
// Notes:
//
// * Unlike other types of plugin, services require macro invocations in
// their headers *and* their compile units.
//
// * In order to declare your service, you must specify your service
// with a scope value of LEGACY, or SHARED as follows:
//
//   * LEGACY services are unaware of multi-threaded operation, and
//     configuration of such a service precludes multi-threaded
//     operation. Such services must have a constructor with one of two
//     signatures:
//
//       MyService(fhicl::ParameterSet const&,
//                 art::ActivityRegistry&);
//
//     or
//
//       MyService(fhicl::ParameterSet const&);
//
//     If the former exists, it will be invoked without ambiguity
//     regardless of the presence or otherwise of the latter.
//
//   * SHARED services are expected to have the same signature as LEGACY
//     services, but warrant additionally that it is safe to call their
//     methods (including callbacks) from multiple threads at the same
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
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/detail/ServiceHandleAllowed.h"
#include "art/Framework/Services/Registry/detail/declaration_helper_macros.h"

// Courtesy include despite not being needed by anything below.
#include "art/Framework/Services/Registry/ServiceTable.h"

// User service macros, Service must provide a constructor with the
// correct signature (see notes above).

// Declare a service.
#define DECLARE_ART_SERVICE(svc, scope) ART_DETAIL_DECLARE_SERVICE(svc, scope)

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
  ART_DETAIL_DECLARE_SERVICE_INTERFACE(svc, scope)

// Declare a service implementing an interface.
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
  ART_DETAIL_DECLARE_SERVICE_INTERFACE_IMPL(svc, iface, scope)

#endif /* art_Framework_Services_Registry_ServiceDeclarationMacros_h */

// Local Variables:
// mode: c++
// End:
