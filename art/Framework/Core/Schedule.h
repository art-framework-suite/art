#ifndef art_Framework_Core_Schedule_h
#define art_Framework_Core_Schedule_h

////////////////////////////////////////////////////////////////////////
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
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunStopwatch.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Utilities/ScheduleID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/trim.h"
#include "cpp0x/functional"
#include "cpp0x/memory"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {
  class ActivityRegistry;
  class TriggerNamesService;

  class Schedule;
}

class art::Schedule {
public:
  Schedule(ScheduleID sID,
           PathManager & pm,
           fhicl::ParameterSet const & processDesc,
           art::TriggerNamesService const & tns,
           MasterProductRegistry & pregistry,
           ActionTable & actions,
           ActivityRegistry & areg);

  template <typename T>
  void processOneOccurrence(typename T::MyPrincipal & principal);

  void beginJob();
  void endJob();

  // Call respondToOpenInputFile() on all Modules
  void respondToOpenInputFile(FileBlock const & fb);

  // Call respondToCloseInputFile() on all Modules
  void respondToCloseInputFile(FileBlock const & fb);

  // Call respondToOpenOutputFiles() on all Modules
  void respondToOpenOutputFiles(FileBlock const & fb);

  // Call respondToCloseOutputFiles() on all Modules
  void respondToCloseOutputFiles(FileBlock const & fb);

  // Temporarily enable or disable a configured path.
  bool setTriggerPathEnabled(std::string const & name, bool enable);

private:
  typedef std::shared_ptr<Worker> WorkerPtr;

  typedef
  std::multimap < std::string,
      cet::exempt_ptr<BranchDescription const> >
      BranchesByModuleLabel;
  typedef
  std::multimap < cet::exempt_ptr<Worker>,
      cet::exempt_ptr<BranchDescription const> >
      OnDemandBranches;

  void resetAll_();

  template <typename T>
  bool runTriggerPaths_(typename T::MyPrincipal &);

  void setupOnDemandSystem_(EventPrincipal & p);

  void makeTriggerResultsInserter_(fhicl::ParameterSet const & trig_pset,
                                   MasterProductRegistry & pregistry,
                                   ActivityRegistry & areg);

  void fillBranchLookup_(ProductList const & pList,
                         BranchesByModuleLabel & branchLookup) const;

  OnDemandBranches catalogOnDemandBranches_(PathManager::Workers && workers,
                                            ProductList const & plist);

  template <class F> void doForAllWorkers_(F fcn);
  template <class F> void doForAllEnabledPaths_(F fcn);

  ScheduleID const sID_;
  fhicl::ParameterSet process_pset_;
  ActionTable    *    act_table_;
  std::string         processName_;
  PathsInfo & triggerPathsInfo_;
  std::vector<unsigned char> pathsEnabled_;
  WorkerPtr      results_inserter_;
  OnDemandBranches demand_branches_;
};

template <typename T>
void
art::Schedule::processOneOccurrence(typename T::MyPrincipal & ep)
{
  this->resetAll_();
  // A RunStopwatch, but only if we are processing an event.
  std::unique_ptr<RunStopwatch>
    stopwatch(triggerPathsInfo_.runStopwatch(T::isEvent_));
  if (T::isEvent_) {
    triggerPathsInfo_.addEvent();
    setupOnDemandSystem_(dynamic_cast<EventPrincipal &>(ep));
  }
  try {
    if (runTriggerPaths_<T>(ep) && T::isEvent_) {
      triggerPathsInfo_.addPass();
    }
    if (results_inserter_.get()) {
      results_inserter_->doWork<T>(ep, 0);
    }
  }
  catch (cet::exception & e) {
    actions::ActionCodes action = (T::isEvent_ ? act_table_->find(
                                     e.root_cause()) : actions::Rethrow);
    assert(action != actions::IgnoreCompletely);
    assert(action != actions::FailPath);
    assert(action != actions::FailModule);
    if (action == actions::SkipEvent) {
      mf::LogWarning(e.category())
        << "an exception occurred and all paths for the event are being skipped: \n"
        << cet::trim_right_copy(e.what(), " \n");
    }
    else
    { throw; }
  }
}

template <typename T>
inline
bool
art::Schedule::runTriggerPaths_(typename T::MyPrincipal & ep)
{
  doForAllEnabledPaths_([&ep](auto p){ p->processOneOccurrence<T>(ep); });
  return triggerPathsInfo_.pathResults().accept();
}

template <class F>
void
art::Schedule::
doForAllWorkers_(F fcn)
{
  for (auto const & val : triggerPathsInfo_.workers()) {
    fcn(val.second.get());
  }
  if (results_inserter_) {
    fcn(results_inserter_.get()); // Do this last -- not part of main list.
  }
}

template <class F>
void
art::Schedule::
doForAllEnabledPaths_(F fcn)
{
  size_t path_index = 0;
  for (auto const & path : triggerPathsInfo_.pathPtrs()) {
    if (pathsEnabled_[path_index++]) {
      fcn(path.get());
    }
  }
}

// ======================================================================

#endif /* art_Framework_Core_Schedule_h */

// Local Variables:
// mode: c++
// End:
