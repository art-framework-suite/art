#ifndef art_Framework_Core_Schedule_h
#define art_Framework_Core_Schedule_h
// vim: set sw=2:

// ======================================================================
// Schedule
//
// A schedule is a sequence of trigger paths. After construction, events
// can be fed to the object and passed through all the modules in the
// schedule. All accounting about processing of events by modules and
// paths is contained here or in object held by containment.
//
// The trigger results producer is generated and managed here. This
// class also manages calls to endjob and beginjob.
//
// A TriggerResults object will always be inserted into the event for
// any schedule. The producer of the TriggerResults EDProduct is always
// the last module in the trigger path. The TriggerResultInserter is
// given a fixed label of "TriggerResults".
//
// Processing of an event happens by pushing the event through the
// Paths. The scheduler performs the reset() on each of the workers
// independent of the Path objects.
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/PrincipalPackages.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/trim.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {

  class ActivityRegistry;
  class MasterProductRegistry;
  class TriggerNamesService;
  class Schedule;

  class Schedule {
  public:
    Schedule(ScheduleID,
             PathManager&,
             fhicl::ParameterSet const&,
             TriggerNamesService const&,
             MasterProductRegistry&,
             ProductDescriptions&,
             ActionTable&,
             ActivityRegistry&);

    template <typename T>
    void process(typename T::MyPrincipal&);

    void beginJob();
    void endJob();

    // Call respondToOpenInputFile() on all Modules
    void respondToOpenInputFile(FileBlock const&);

    // Call respondToCloseInputFile() on all Modules
    void respondToCloseInputFile(FileBlock const&);

    // Call respondToOpenOutputFiles() on all Modules
    void respondToOpenOutputFiles(FileBlock const&);

    // Call respondToCloseOutputFiles() on all Modules
    void respondToCloseOutputFiles(FileBlock const&);

  private:
    // Private initialization helpers.
    void makeTriggerResultsInserter_(fhicl::ParameterSet const& trig_pset,
                                     MasterProductRegistry& mpr,
                                     ProductDescriptions& productsToProduce,
                                     ActivityRegistry& areg);

    template <typename T>
    bool runTriggerPaths_(typename T::MyPrincipal&);

    template <class F>
    void doForAllWorkers_(F functor);

    template <class F>
    void doForAllEnabledPaths_(F functor);

    // Data members.
    ScheduleID const sID_;
    fhicl::ParameterSet process_pset_;
    ActionTable* act_table_;
    std::string processName_;
    PathsInfo& triggerPathsInfo_;
    std::vector<unsigned char> pathsEnabled_;
    std::unique_ptr<Worker> results_inserter_{nullptr};
  };

  template <typename T>
  void
  Schedule::process(typename T::MyPrincipal& principal)
  {
    doForAllWorkers_([](auto w) { w->reset(); });
    triggerPathsInfo_.pathResults().reset();
    if (T::level == Level::Event) {
      triggerPathsInfo_.addEvent();
    }
    try {
      if (runTriggerPaths_<T>(principal) && T::level == Level::Event) {
        triggerPathsInfo_.addPass();
      }
      if (results_inserter_.get()) {
        results_inserter_->doWork<T>(principal, 0);
      }
    }
    catch (cet::exception& e) {
      actions::ActionCodes const action = {T::level == Level::Event ?
                                             act_table_->find(e.root_cause()) :
                                             actions::Rethrow};
      assert(action != actions::IgnoreCompletely);
      assert(action != actions::FailPath);
      assert(action != actions::FailModule);
      if (action == actions::SkipEvent) {
        mf::LogWarning(e.category())
          << "an exception occurred and all paths for "
             "the event are being skipped: \n"
          << cet::trim_right_copy(e.what(), " \n");
      } else {
        throw;
      }
    }
  }

  template <typename T>
  inline bool
  Schedule::runTriggerPaths_(typename T::MyPrincipal& ep)
  {
    doForAllEnabledPaths_([&ep](auto p) { p->template process<T>(ep); });
    return triggerPathsInfo_.pathResults().accept();
  }

  template <class F>
  void
  Schedule::doForAllWorkers_(F functor)
  {
    for (auto const& val : triggerPathsInfo_.workers()) {
      functor(val.second.get());
    }
    if (results_inserter_) {
      // Do this last -- not part of main list.
      functor(results_inserter_.get());
    }
  }

  template <class F>
  void
  Schedule::doForAllEnabledPaths_(F functor)
  {
    size_t path_index = 0;
    for (auto const& path : triggerPathsInfo_.pathPtrs()) {
      if (pathsEnabled_[path_index++]) {
        functor(path.get());
      }
    }
  }

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Core_Schedule_h */
