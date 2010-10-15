#ifndef FWCore_Framework_OccurrenceTraits_h
#define FWCore_Framework_OccurrenceTraits_h

/*----------------------------------------------------------------------

OccurrenceTraits:

----------------------------------------------------------------------*/


#include "art/Framework/Core/BranchActionType.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Provenance/ModuleDescription.h"


namespace art {
  template <typename T, BranchActionType B> class OccurrenceTraits;

  template <>
  class OccurrenceTraits<EventPrincipal, BranchActionBegin> {
  public:
    typedef EventPrincipal MyPrincipal;
    static bool const begin_ = true;
    static bool const isEvent_ = true;
    static void preScheduleSignal(ActivityRegistry *a, EventPrincipal const* ep) {
      a->preProcessEventSignal_(ep->id(), ep->time());
    }
    static void postScheduleSignal(ActivityRegistry *a, EventPrincipal* ep) {
      Event ev(*ep, ModuleDescription());
      a->postProcessEventSignal_(ev);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->preProcessPathSignal_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->postProcessPathSignal_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->preModuleSignal_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->postModuleSignal_(*md);
    }
  };

  template <>
  class OccurrenceTraits<RunPrincipal, BranchActionBegin> {
  public:
    typedef RunPrincipal MyPrincipal;
    static bool const begin_ = true;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, RunPrincipal const* ep) {
      a->preBeginRunSignal_(ep->id(), ep->beginTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->postBeginRunSignal_(run);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->prePathBeginRunSignal_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->postPathBeginRunSignal_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->preModuleBeginRunSignal_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->postModuleBeginRunSignal_(*md);
    }
  };

  template <>
  class OccurrenceTraits<RunPrincipal, BranchActionEnd> {
  public:
    typedef RunPrincipal MyPrincipal;
    static bool const begin_ = false;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, RunPrincipal const* ep) {
      a->preEndRunSignal_(ep->id(), ep->endTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, RunPrincipal* ep) {
      Run run(*ep, ModuleDescription());
      a->postEndRunSignal_(run);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->prePathEndRunSignal_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->postPathEndRunSignal_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->preModuleEndRunSignal_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->postModuleEndRunSignal_(*md);
    }
  };

  template <>
  class OccurrenceTraits<SubRunPrincipal, BranchActionBegin> {
  public:
    typedef SubRunPrincipal MyPrincipal;
    static bool const begin_ = true;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, SubRunPrincipal const* ep) {
      a->preBeginSubRunSignal_(ep->id(), ep->beginTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, SubRunPrincipal* ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->postBeginSubRunSignal_(subRun);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->prePathBeginSubRunSignal_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->postPathBeginSubRunSignal_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->preModuleBeginSubRunSignal_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->postModuleBeginSubRunSignal_(*md);
    }
  };

  template <>
  class OccurrenceTraits<SubRunPrincipal, BranchActionEnd> {
  public:
    typedef SubRunPrincipal MyPrincipal;
    static bool const begin_ = false;
    static bool const isEvent_ = false;
    static void preScheduleSignal(ActivityRegistry *a, SubRunPrincipal const* ep) {
      a->preEndSubRunSignal_(ep->id(), ep->beginTime());
    }
    static void postScheduleSignal(ActivityRegistry *a, SubRunPrincipal* ep) {
      SubRun subRun(*ep, ModuleDescription());
      a->postEndSubRunSignal_(subRun);
    }
    static void prePathSignal(ActivityRegistry *a, std::string const& s) {
      a->prePathEndSubRunSignal_(s);
    }
    static void postPathSignal(ActivityRegistry *a, std::string const& s, HLTPathStatus const& status) {
      a->postPathEndSubRunSignal_(s, status);
    }
    static void preModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->preModuleEndSubRunSignal_(*md);
    }
    static void postModuleSignal(ActivityRegistry *a, ModuleDescription const* md) {
      a->postModuleEndSubRunSignal_(*md);
    }
  };
}

#endif  // FWCore_Framework_OccurrenceTraits_h
