#ifndef art_Framework_Principal_OccurrenceTraits_h
#define art_Framework_Principal_OccurrenceTraits_h

// ======================================================================
//
// OccurrenceTraits
//
// ======================================================================

#include "art/Framework/Services/Registry/BranchActionType.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

// ----------------------------------------------------------------------

namespace art {
  template <typename T, BranchActionType B> class OccurrenceTraits;

  template <>
  class OccurrenceTraits<EventPrincipal, BranchActionBegin> {
  public:
    using MyPrincipal = EventPrincipal;
    constexpr static bool begin_ {true};
    constexpr static bool isEvent_ {true};
    static void preScheduleSignal(ActivityRegistry *a, EventPrincipal * ep) {
      Event ev(*ep, ModuleDescription());
      a->sPreProcessEvent.invoke(ev);
    }
    static void postScheduleSignal(ActivityRegistry *a, EventPrincipal* ep) {
      Event ev(*ep, ModuleDescription());
      a->sPostProcessEvent.invoke(ev);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPreProcessPath.invoke(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostProcessPath.invoke(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModule.invoke(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModule.invoke(*md);
    }
  };

  template <>
  class OccurrenceTraits<RunPrincipal, BranchActionBegin> {
  public:
    using MyPrincipal = RunPrincipal;
    constexpr static bool begin_ {true};
    constexpr static bool isEvent_ {false};
    static void preScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->sPreBeginRun.invoke(run);
    }
    static void postScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->sPostBeginRun.invoke(run);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathBeginRun.invoke(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathBeginRun.invoke(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleBeginRun.invoke(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleBeginRun.invoke(*md);
    }
  };

  template <>
  class OccurrenceTraits<RunPrincipal, BranchActionEnd> {
  public:
    using MyPrincipal = RunPrincipal;
    constexpr static bool begin_ {false};
    constexpr static bool isEvent_ {false};
    static void preScheduleSignal(ActivityRegistry *a, RunPrincipal const* ep) {
      a->sPreEndRun.invoke(ep->id(), ep->endTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->sPostEndRun.invoke(run);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathEndRun.invoke(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathEndRun.invoke(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleEndRun.invoke(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleEndRun.invoke(*md);
    }
  };

  template <>
  class OccurrenceTraits<SubRunPrincipal, BranchActionBegin> {
  public:
    using MyPrincipal = SubRunPrincipal;
    constexpr static bool begin_ {true};
    constexpr static bool isEvent_ {false};
    static void preScheduleSignal(ActivityRegistry *a, SubRunPrincipal * ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->sPreBeginSubRun.invoke(subRun);
    }
    static void postScheduleSignal(ActivityRegistry *a, SubRunPrincipal* ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->sPostBeginSubRun.invoke(subRun);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathBeginSubRun.invoke(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathBeginSubRun.invoke(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleBeginSubRun.invoke(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleBeginSubRun.invoke(*md);
    }
  };

  template <>
  class OccurrenceTraits<SubRunPrincipal, BranchActionEnd> {
  public:
    using MyPrincipal = SubRunPrincipal;
    constexpr static bool begin_ {false};
    constexpr static bool isEvent_ {false};
    static void preScheduleSignal(ActivityRegistry *a, SubRunPrincipal const* ep) {
      a->sPreEndSubRun.invoke(ep->id(), ep->beginTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, SubRunPrincipal* ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->sPostEndSubRun.invoke(subRun);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathEndSubRun.invoke(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathEndSubRun.invoke(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleEndSubRun.invoke(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleEndSubRun.invoke(*md);
    }
  };
}

// ======================================================================

#endif /* art_Framework_Principal_OccurrenceTraits_h */

// Local Variables:
// mode: c++
// End:
