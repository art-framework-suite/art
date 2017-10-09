#ifndef art_Framework_Principal_PrincipalPackages_h
#define art_Framework_Principal_PrincipalPackages_h

// ======================================================================
//
// PrincipalPackages: encapsulates information necessary for
//                    processing of Events, SubRuns, and Runs.
//
// ======================================================================

#include "art/Framework/Principal/BranchActionType.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"

// ----------------------------------------------------------------------

namespace art {

  template <Level L>
  class ProcessPackage {
  public:
    static_assert(L == Level::Event,
                  "ProcessPackage can be used only for the Event level.\n");
    using MyPrincipal = EventPrincipal;
    constexpr static BranchActionType processing_action{BranchActionProcess};
    constexpr static Level level{L};
    static void
    preScheduleSignal(ActivityRegistry& a, EventPrincipal const& ep)
    {
      Event const ev{ep, ModuleDescription{}, Consumer::non_module_context()};
      a.sPreProcessEvent.invoke(ev);
    }
    static void
    postScheduleSignal(ActivityRegistry& a, EventPrincipal const& ep)
    {
      Event const ev{ep, ModuleDescription{}, Consumer::non_module_context()};
      a.sPostProcessEvent.invoke(ev);
    }
    static void
    prePathSignal(ActivityRegistry& a, std::string const& s)
    {
      a.sPreProcessPath.invoke(s);
    }
    static void
    postPathSignal(ActivityRegistry& a,
                   std::string const& s,
                   HLTPathStatus const& status)
    {
      a.sPostProcessPath.invoke(s, status);
    }
    static void
    preModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
    {
      a.sPreModule.invoke(md);
    }
    static void
    postModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
    {
      a.sPostModule.invoke(md);
    }
  };

  template <Level L>
  using Do = ProcessPackage<L>;

  template <Level>
  class BeginEndPackage;

  template <>
  class BeginEndPackage<Level::Run> {
  public:
    class Begin {
    public:
      using MyPrincipal = RunPrincipal;
      constexpr static Level level{Level::Run};
      constexpr static BranchActionType processing_action{BranchActionBegin};

      static void
      preScheduleSignal(ActivityRegistry& a, RunPrincipal const& rp)
      {
        Run const run{rp, ModuleDescription{}, Consumer::non_module_context()};
        a.sPreBeginRun.invoke(run);
      }
      static void
      postScheduleSignal(ActivityRegistry& a, RunPrincipal const& rp)
      {
        Run const run{rp, ModuleDescription{}, Consumer::non_module_context()};
        a.sPostBeginRun.invoke(run);
      }
      static void
      prePathSignal(ActivityRegistry& a, std::string const& s)
      {
        a.sPrePathBeginRun.invoke(s);
      }
      static void
      postPathSignal(ActivityRegistry& a,
                     std::string const& s,
                     HLTPathStatus const& status)
      {
        a.sPostPathBeginRun.invoke(s, status);
      }
      static void
      preModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPreModuleBeginRun.invoke(md);
      }
      static void
      postModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPostModuleBeginRun.invoke(md);
      }
    };

    class End {
    public:
      using MyPrincipal = RunPrincipal;
      constexpr static Level level{Level::Run};
      constexpr static BranchActionType processing_action{BranchActionEnd};

      static void
      preScheduleSignal(ActivityRegistry& a, RunPrincipal const& rp)
      {
        a.sPreEndRun.invoke(rp.id(), rp.endTime());
      }
      static void
      postScheduleSignal(ActivityRegistry& a, RunPrincipal const& rp)
      {
        Run const run{rp, ModuleDescription{}, Consumer::non_module_context()};
        a.sPostEndRun.invoke(run);
      }
      static void
      prePathSignal(ActivityRegistry& a, std::string const& s)
      {
        a.sPrePathEndRun.invoke(s);
      }
      static void
      postPathSignal(ActivityRegistry& a,
                     std::string const& s,
                     HLTPathStatus const& status)
      {
        a.sPostPathEndRun.invoke(s, status);
      }
      static void
      preModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPreModuleEndRun.invoke(md);
      }
      static void
      postModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPostModuleEndRun.invoke(md);
      }
    };
  };

  template <>
  class BeginEndPackage<Level::SubRun> {
  public:
    class Begin {
    public:
      using MyPrincipal = SubRunPrincipal;
      constexpr static Level level{Level::SubRun};
      constexpr static BranchActionType processing_action{BranchActionBegin};

      static void
      preScheduleSignal(ActivityRegistry& a, SubRunPrincipal const& srp)
      {
        SubRun const sr{
          srp, ModuleDescription{}, Consumer::non_module_context()};
        a.sPreBeginSubRun.invoke(sr);
      }
      static void
      postScheduleSignal(ActivityRegistry& a, SubRunPrincipal const& srp)
      {
        SubRun const sr{
          srp, ModuleDescription{}, Consumer::non_module_context()};
        a.sPostBeginSubRun.invoke(sr);
      }
      static void
      prePathSignal(ActivityRegistry& a, std::string const& s)
      {
        a.sPrePathBeginSubRun.invoke(s);
      }
      static void
      postPathSignal(ActivityRegistry& a,
                     std::string const& s,
                     HLTPathStatus const& status)
      {
        a.sPostPathBeginSubRun.invoke(s, status);
      }
      static void
      preModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPreModuleBeginSubRun.invoke(md);
      }
      static void
      postModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPostModuleBeginSubRun.invoke(md);
      }
    };

    class End {
    public:
      using MyPrincipal = SubRunPrincipal;
      constexpr static Level level{Level::SubRun};
      constexpr static BranchActionType processing_action{BranchActionEnd};

      static void
      preScheduleSignal(ActivityRegistry& a, SubRunPrincipal const& srp)
      {
        a.sPreEndSubRun.invoke(srp.id(), srp.endTime());
      }
      static void
      postScheduleSignal(ActivityRegistry& a, SubRunPrincipal const& srp)
      {
        SubRun const sr{
          srp, ModuleDescription{}, Consumer::non_module_context()};
        a.sPostEndSubRun.invoke(sr);
      }
      static void
      prePathSignal(ActivityRegistry& a, std::string const& s)
      {
        a.sPrePathEndSubRun.invoke(s);
      }
      static void
      postPathSignal(ActivityRegistry& a,
                     std::string const& s,
                     HLTPathStatus const& status)
      {
        a.sPostPathEndSubRun.invoke(s, status);
      }
      static void
      preModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPreModuleEndSubRun.invoke(md);
      }
      static void
      postModuleSignal(ActivityRegistry& a, ModuleDescription const& md)
      {
        a.sPostModuleEndSubRun.invoke(md);
      }
    };
  };

  template <Level L>
  using Begin = typename BeginEndPackage<L>::Begin;

  template <Level L>
  using End = typename BeginEndPackage<L>::End;
}

  // ======================================================================

#endif /* art_Framework_Principal_PrincipalPackages_h */

// Local Variables:
// mode: c++
// End:
