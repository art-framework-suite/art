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

namespace {
  template <class T> class ScheduleSignalSentry;
}

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
           std::shared_ptr<ActivityRegistry> areg);

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

  void resetAll();

  template <typename T>
  bool runTriggerPaths(typename T::MyPrincipal &);

  void setupOnDemandSystem(EventPrincipal & p);

  void makeTriggerResultsInserter(fhicl::ParameterSet const & trig_pset,
                                  MasterProductRegistry & pregistry);

  void fillBranchLookup(ProductList const & pList,
                        BranchesByModuleLabel & branchLookup) const;

  void catalogOnDemandBranches(BranchesByModuleLabel const & branchLookup);

  void doForAllWorkers_(std::function<void (Worker *)> func);
  void doForAllPaths_(std::function<void (Path *)> func);

  ScheduleID const sID_;
  fhicl::ParameterSet process_pset_;
  ActionTable    *    act_table_;
  std::string         processName_;
  std::shared_ptr<ActivityRegistry> actReg_;

  PathsInfo & triggerPathsInfo_;

  WorkerPtr      results_inserter_;
  OnDemandBranches demand_branches_;
};

template <typename T>
void
art::Schedule::processOneOccurrence(typename T::MyPrincipal & ep)
{
  this->resetAll();
  // A RunStopwatch, but only if we are processing an event.
  std::unique_ptr<RunStopwatch>
    stopwatch(triggerPathsInfo_.runStopwatch(T::isEvent));
  if (T::isEvent_) {
    triggerPathsInfo_.addEvent();
    setupOnDemandSystem(dynamic_cast<EventPrincipal &>(ep));
  }
  try {
    ScheduleSignalSentry<T> sentry(*actReg_, ep);
    try {
      if (runTriggerPaths<T>(ep) && T::isEvent_) {
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
  catch (cet::exception & ex) {
    actions::ActionCodes action = (T::isEvent_ ? act_table_->find(
                                     ex.root_cause()) : actions::Rethrow);
    switch (action) {
      case actions::IgnoreCompletely: {
        mf::LogWarning(ex.category())
            << "exception being ignored for current event:\n"
            << cet::trim_right_copy(ex.what(), " \n");
        break;
      }
      default: {
        throw art::Exception(errors::EventProcessorFailure)
            << "An exception occurred during current event processing\n"
            << ex;
      }
    }
  }
  catch (...) {
    mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
    throw;
  }
}

template <typename T>
bool
art::Schedule::runTriggerPaths(typename T::MyPrincipal & ep)
{
  doForAllPaths_(std::bind(&Path::processOneOccurrence<T>,
                           std::placeholders::_1,
                           std::ref(ep)));
  return triggerPathsInfo_.pathResults().accept();
}

namespace {

  /// Class ScheduleSignalSentry<T> is used to emit the pre- and post-schedule
  /// signals associated with the principal associated with T.
  template <typename T>
  class ScheduleSignalSentry {
  public:
    ScheduleSignalSentry(ScheduleSignalSentry<T> const &) = delete;
    ScheduleSignalSentry<T> operator=(ScheduleSignalSentry<T> const &) = delete;

    typedef typename T::MyPrincipal principal_t;
    ScheduleSignalSentry(art::ActivityRegistry & a, principal_t & ep);
    ~ScheduleSignalSentry();

  private:
    art::ActivityRegistry & a_;
    principal_t      &      ep_;
  };

  template <class T>
  ScheduleSignalSentry<T>::ScheduleSignalSentry(art::ActivityRegistry & a,
                                                typename ScheduleSignalSentry<T>::principal_t & ep) :
    a_(a),
    ep_(ep)
  {
    T::preScheduleSignal(&a_, &ep_);
  }

  template <class T>
  ScheduleSignalSentry<T>::~ScheduleSignalSentry()
  {
    T::postScheduleSignal(&a_, &ep_);
  }
}

// ======================================================================

#endif /* art_Framework_Core_Schedule_h */

// Local Variables:
// mode: c++
// End:
