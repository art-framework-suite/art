#ifndef art_Framework_Services_Registry_ServiceMacros_h
#define art_Framework_Services_Registry_ServiceMacros_h

////////////////////////////////////////////////////////////////////////
// ServiceMacros : define the macros used to declare an ART service.
//
// All user services:
// DEFINE_ART_{GLOBAL,PERSCHEDULE}_SERVICE
//
// Some system services (special construction).
// DEFINE_ART_{GLOBAL,PERSCHEDULE}_SYSTEM_SERVICE
//
// All user services must have one of the following two constructors
// depending on their type:
//
// GLOBAL:
// MyService(fhicl::ParameterSet const &, art::ActivityRegistry &);
//
// PERSCHEDULE:
// MyService(fhicl::ParameterSet const &,
//           art::ActivityRegistry &,
//           art::ScheduleID);
//
// GLOBAL services *must* be safe against calls from multiple schedules
// at the same time, but may register for callbacks for per-schedule
// signals (however the same callback must be registered for all
// schedules per the ActivityRegistry interface).
//
// PERSCHEDULE services may register for global callbacks, but there
// will be n unique instances of the service (and therefore unique
// callbacks), one for each schedule.
//
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServiceWrapperBase.h"
#include "art/Utilities/TypeID.h"
#include "cpp0x/memory"

////////////////////////////////////////////////////////////////////////
// Helper macros.
#define DEFINE_ART_SERVICE_HOOK(klass)          \
  extern "C"                                    \
  art::TypeID                                   \
  get_typeid()                                  \
  { return art::TypeID(typeid(klass)); }

#define DEFINE_ART_SERVICE_SCOPE(scope)         \
  extern "C" art::ServiceScope service_scope()  \
  {                                             \
    return art::ServiceScope::scope;            \
  }

#define DEFINE_ART_GLOBAL_SERVICE_INT(klass)                            \
  DEFINE_ART_SERVICE_SCOPE(GLOBAL)                                      \
  extern "C" std::auto_ptr<art::ServiceWrapperBase>                     \
  make(fhicl::ParameterSet const & cfg, art::ActivityRegistry & reg)    \
  {                                                                     \
    return std::auto_ptr<art::ServiceWrapperBase>                       \
           (new art::ServiceWrapper<klass>                                   \
            (std::auto_ptr<klass>(new klass(cfg,reg))                        \
            )                                                                \
           );                                                                \
  }

#define DEFINE_ART_PERSCHEDULE_SERVICE_INT(klass)                       \
  DEFINE_ART_SERVICE_SCOPE(PER_SCHEDULE)                                \
  extern "C" std::vector<std::unique_ptr<art::ServiceWrapperBase>>      \
      make(fhicl::ParameterSet const & cfg,                                 \
           art::ActivityRegistry & reg,                                     \
           size_t numSchedules)                                             \
  {                                                                     \
    std::vector<std::unique_ptr<art::ServiceWrapperBase>> result;       \
    result.reserve(numSchedules);                                       \
    for (size_t sID = 0; sID < numSchedules; ++sID)                     \
    {                                                                   \
      result.emplace_back(new art::ServiceWrapper<klass>                \
                          (std::auto_ptr<klass>                         \
                           (new klass(cfg, reg, ScheduleID(sID)))       \
                          )                                             \
                         );                                             \
    }                                                                   \
  }

////////////////////////////////////////////////////////////////////////
// New macros
#define DEFINE_ART_GLOBAL_SERVICE(klass)         \
  DEFINE_ART_GLOBAL_SERVICE_INT(klass)           \
  DEFINE_ART_SERVICE_HOOK(klass)

#define DEFINE_ART_GLOBAL_SYSTEM_SERVICE(klass) \
  DEFINE_ART_SERVICE_SCOPE(GLOBAL)              \
  DEFINE_ART_SERVICE_HOOK(klass)

#define DEFINE_ART_PERSCHEDULE_SERVICE(klass) \
  DEFINE_ART_PERSCHEDULE_SERVICE_INT(klass)   \
  DEFINE_ART_SERVICE_HOOK(klass)

#define DEFINE_ART_PERSCHEDULE_SYSTEM_SERVICE(klass) \
  DEFINE_ART_SERVICE_SCOPE(PER_SCHEDULE)             \
  DEFINE_ART_SERVICE_HOOK(klass)

////////////////////////////////////////////////////////////////////////
// Legacy macros
#define DEFINE_ART_SYSTEM_SERVICE(klass)        \
  DEFINE_ART_GLOBAL_SYSTEM_SERVICE(klass)

#define DEFINE_ART_SERVICE(klass)               \
  DEFINE_ART_GLOBAL_SERVICE(klass)

#endif /* art_Framework_Services_Registry_ServiceMacros_h */

// Local Variables:
// mode: c++
// End:
