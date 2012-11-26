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
#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

// ----------------------------------------------------------------------

namespace art {
  template <typename T, BranchActionType B> class OccurrenceTraits;

  template <>
  class OccurrenceTraits<EventPrincipal, BranchActionBegin> {
  public:
    typedef EventPrincipal MyPrincipal;
    static bool const begin_ = true;
    static bool const isEvent_ = true;
    static void preScheduleSignal(ActivityRegistry *a, EventPrincipal * ep) {
      Event ev(*ep, ModuleDescription());
      a->sPreProcessEvent.invoke(ev);
    }
    static void postScheduleSignal(ActivityRegistry *a, EventPrincipal* ep) {
      Event ev(*ep, ModuleDescription());
      a->sPostProcessEvent_(ev);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPreProcessPath_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostProcessPath_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModule_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModule_(*md);
    }
  };

  template <>
  class OccurrenceTraits<RunPrincipal, BranchActionBegin> {
  public:
    typedef RunPrincipal MyPrincipal;
    static bool const begin_ = true;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->sPreBeginRun.invoke(run);
    }
    static void postScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->sPostBeginRun_(run);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathBeginRun_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathBeginRun_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleBeginRun_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleBeginRun_(*md);
    }
  };

  template <>
  class OccurrenceTraits<RunPrincipal, BranchActionEnd> {
  public:
    typedef RunPrincipal MyPrincipal;
    static bool const begin_ = false;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, RunPrincipal const* ep) {
      a->sPreEndRun.invoke(ep->id(), ep->endTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->sPostEndRun_(run);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathEndRun_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathEndRun_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleEndRun_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleEndRun_(*md);
    }
  };

  template <>
  class OccurrenceTraits<SubRunPrincipal, BranchActionBegin> {
  public:
    typedef SubRunPrincipal MyPrincipal;
    static bool const begin_ = true;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, SubRunPrincipal * ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->sPreBeginSubRun.invoke(subRun);
    }
    static void postScheduleSignal(ActivityRegistry *a, SubRunPrincipal* ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->sPostBeginSubRun_(subRun);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathBeginSubRun_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathBeginSubRun_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleBeginSubRun_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleBeginSubRun_(*md);
    }
  };

  template <>
  class OccurrenceTraits<SubRunPrincipal, BranchActionEnd> {
  public:
    typedef SubRunPrincipal MyPrincipal;
    static bool const begin_ = false;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, SubRunPrincipal const* ep) {
      a->sPreEndSubRun.invoke(ep->id(), ep->beginTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, SubRunPrincipal* ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->sPostEndSubRun_(subRun);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->sPrePathEndSubRun_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->sPostPathEndSubRun_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPreModuleEndSubRun_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->sPostModuleEndSubRun_(*md);
    }
  };
}

// ======================================================================

#endif /* art_Framework_Principal_OccurrenceTraits_h */

// Local Variables:
// mode: c++
// End:
