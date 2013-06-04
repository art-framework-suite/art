#ifndef art_Framework_Core_Schedule_h
#define art_Framework_Core_Schedule_h

// ======================================================================
/*
  A class for creating a schedule based on paths in the configuration file.
  The schedule is maintained as a sequence of paths.
  After construction, events can be fed to the object and passed through
  all the modules in the schedule.  All accounting about processing
  of events by modules and paths is contained here or in object held
  by containment.

  The trigger results producer and product are generated and managed here.
  This class also manages endpaths and calls to endjob and beginjob.
  Endpaths are just treated as a simple list of modules that need to
  do processing of the event and do not participate in trigger path
  activities.

  This class requires the high-level process pset.  It uses process_name.
  If the "services" pset contains an "scheduler" pset, then the
  following optional parameter can be present:
  bool wantSummary = true/false   # default false

  wantSummary indicates whether or not the pass/fail/error stats
  for modules and paths should be printed at the end-of-job.

  A TriggerResults object will always be inserted into the event
  for any schedule.  The producer of the TriggerResults EDProduct
  is always the first module in the endpath.  The TriggerResultInserter
  is given a fixed label of "TriggerResults".

  The Schedule throws an exception if any EventObservers are present
  in a path: they belong in an end path

  The Schedule issues a warning if a ProducerBase is present in an
  end path: it belongs in a path.

  Processing of an event happens by pushing the event through the Paths.
  The scheduler performs the reset() on each of the workers independent
  of the Path objects.

  ------------------------

  About Paths:
  Paths fit into two categories:
  1) trigger paths that contribute directly to saved trigger bits
  2) end paths
  The Schedule holds these paths in two data structures:
  1) main path list
  2) end path list

  Trigger path processing always precedes endpath processing.
  The order of the paths from the input configuration is
  preserved in the main paths list.

  ------------------------

  The Schedule uses the TriggerNamesService to get the names of the
  trigger paths and end paths. When a TriggerResults object is created
  the results are stored in the same order as the trigger names from
  TriggerNamesService.

*/
// ======================================================================

#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Core/Path.h"
#include "art/Framework/Principal/RunStopwatch.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/trim.h"
#include "cpp0x/memory"
#include "cpp0x/utility"
#include "cpp0x/functional"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <map>
#include <set>
#include <string>
#include <vector>

// ----------------------------------------------------------------------
namespace {
  template <class T> class ScheduleSignalSentry;
}

namespace art {
  class ActivityRegistry;
  class OutputWorker;
  class TriggerNamesService;
  class WorkerRegistry;

  class Schedule {
    typedef std::vector<std::string> vstring;
    typedef std::vector<Path> Paths;
    typedef std::shared_ptr<HLTGlobalStatus> TrigResPtr;
    typedef std::shared_ptr<Worker> WorkerPtr;
    typedef std::vector<OutputWorker *> OutputWorkers;
    typedef std::vector<WorkerInPath> PathWorkers;

  public:
    typedef std::vector<Worker *> Workers;

    Schedule(fhicl::ParameterSet const & processDesc,
             art::TriggerNamesService const & tns,
             WorkerRegistry & wregistry,
             MasterProductRegistry & pregistry,
             ActionTable & actions,
             std::shared_ptr<ActivityRegistry> areg);



    template <typename T>
    void processOneOccurrence(typename T::MyPrincipal & principal);

    void beginJob();
    void endJob();

    // Write the subRun
    void writeSubRun(SubRunPrincipal const & srp);

    // Write the run
    void writeRun(RunPrincipal const & rp);

    // Call closeFile() on all OutputModules.
    void closeOutputFiles();

    // Call openNewFileIfNeeded() on all OutputModules
    void openNewOutputFilesIfNeeded();

    // Call openFiles() on all OutputModules
    void openOutputFiles(FileBlock & fb);

    // Call respondToOpenInputFile() on all Modules
    void respondToOpenInputFile(FileBlock const & fb);

    // Call respondToCloseInputFile() on all Modules
    void respondToCloseInputFile(FileBlock const & fb);

    // Call respondToOpenOutputFiles() on all Modules
    void respondToOpenOutputFiles(FileBlock const & fb);

    // Call respondToCloseOutputFiles() on all Modules
    void respondToCloseOutputFiles(FileBlock const & fb);

    // Call shouldWeCloseFile() on all OutputModules.
    bool shouldWeCloseOutput() const;

    std::pair<double, double> timeCpuReal() const;

    /// Return a vector allowing const access to all the
    /// ModuleDescriptions for this Schedule.

    /// *** N.B. *** Ownership of the ModuleDescriptions is *not*
    /// *** passed to the caller. Do not call delete on these
    /// *** pointers!
    std::vector<ModuleDescription const *> getAllModuleDescriptions() const;

    /// Return the number of events this Schedule has tried to process
    /// (inclues both successes and failures, including failures due
    /// to exceptions during processing).
    int totalEvents() const;

    /// Return the number of events which have been passed by one or
    /// more trigger paths.
    int totalEventsPassed() const;

    /// Return the number of events that have not passed any trigger.
    /// (N.B. totalEventsFailed() + totalEventsPassed() == totalEvents()
    int totalEventsFailed() const;

    /// Return the trigger report information on paths,
    /// modules-in-path, modules-in-endpath, and modules.
    void getTriggerReport(TriggerReport & rep) const;

    /// Return whether a module has reached its maximum count.
    bool terminate() const;

    ///  Clear all the counters in the trigger report.
    void clearCounters();

    // Retrieve all workers.
    void getAllWorkers(Workers & out);

  private:
    typedef std::vector<cet::exempt_ptr<Worker> > OnDemandWorkers;
    typedef
    std::multimap < std::string,
        cet::exempt_ptr<BranchDescription const> >
        BranchesByModuleLabel;
    typedef
    std::multimap < cet::exempt_ptr<Worker>,
        cet::exempt_ptr<BranchDescription const> >
        OnDemandBranches;

    void writeSummary();

    Workers::const_iterator workersBegin() const { return all_workers_.begin(); }

    Workers::const_iterator workersEnd() const { return all_workers_.end(); }

    Workers::iterator workersBegin() { return  all_workers_.begin(); }

    Workers::iterator workersEnd() { return all_workers_.end(); }

    void resetAll();

    template <typename T>
    bool runTriggerPaths(typename T::MyPrincipal &);

    template <typename T>
    void runEndPaths(typename T::MyPrincipal &);

    void setupOnDemandSystem(EventPrincipal & p);

    void reportSkipped(EventPrincipal const & ep) const;
    void reportSkipped(SubRunPrincipal const &) const;
    void reportSkipped(RunPrincipal const &) const;

    void fillWorkers(std::string const & name, PathWorkers & out, bool IsTrigPath, MasterProductRegistry & pregistry);
    void fillTrigPath(int bitpos, std::string const & name, TrigResPtr trptr, MasterProductRegistry & pregistry);
    void fillEndPath(int bitpos, std::string const & name, MasterProductRegistry & pregistry);

    void limitOutput();

    void addToAllWorkers(Worker * w);

    void makeTriggerResultsInserter(fhicl::ParameterSet const & trig_pset, MasterProductRegistry & pregistry);

    void fillBranchLookup(ProductList const & pList,
                          BranchesByModuleLabel & branchLookup) const;

    void catalogOnDemandBranches(OnDemandWorkers const & odw,
                                 BranchesByModuleLabel const & branchLookup);
    void catalogOneOnDemandWorker(cet::exempt_ptr<Worker> wp,
                                  BranchesByModuleLabel const & branchLookup);

    void pathConsistencyCheck(size_t expected_num_workers) const;

    size_t checkOnePath(Path const & path, bool isEndPath) const;

    size_t accumulateConsistencyFailures(size_t current_num_failures,
                                         art::Path const & path,
                                         bool isEndPath) const;

    fhicl::ParameterSet process_pset_;
    WorkerRegistry   *  worker_reg_;
    ActionTable    *    act_table_;
    std::string         processName_;
    std::shared_ptr<ActivityRegistry> actReg_;

    vstring trig_name_list_;
    vstring end_path_name_list_;

    TrigResPtr   results_;
    TrigResPtr   endpath_results_;

    WorkerPtr      results_inserter_;
    Workers        all_workers_;
    OutputWorkers  all_output_workers_;
    Paths          trig_paths_;
    Paths          end_paths_;
    OnDemandBranches demand_branches_;

    bool                           wantSummary_;
    int                            total_events_;
    int                            total_passed_;
    RunStopwatch::StopwatchPointer stopwatch_;
  };

  inline
  std::pair<double, double> Schedule::timeCpuReal() const
  {
    return std::make_pair(stopwatch_->cpuTime(),
                          stopwatch_->realTime());
  }

  inline
  int Schedule::totalEvents() const
  {
    return total_events_;
  }

  inline
  int Schedule::totalEventsPassed() const
  {
    return total_passed_;
  }

  inline
  int Schedule::totalEventsFailed() const
  {
    return totalEvents() - totalEventsPassed();
  }

  void
  inline
  Schedule::reportSkipped(EventPrincipal const &) const
  {
  }

  void
  inline
  Schedule::reportSkipped(SubRunPrincipal const &) const
  {
  }

  void
  inline
  Schedule::reportSkipped(RunPrincipal const &) const
  {
  }

  template <typename T>
  void
  Schedule::processOneOccurrence(typename T::MyPrincipal & ep)
  {
    this->resetAll();
    // A RunStopwatch, but only if we are processing an event.
    std::unique_ptr<RunStopwatch> stopwatch(T::isEvent_ ? new RunStopwatch(stopwatch_) : 0);
    if (T::isEvent_) {
      ++total_events_;
      setupOnDemandSystem(dynamic_cast<EventPrincipal &>(ep));
    }
    try {
      ScheduleSignalSentry<T> sentry(*actReg_, ep);
      try {
        if (runTriggerPaths<T>(ep) && T::isEvent_)  { ++total_passed_; }
        if (results_inserter_.get()) { results_inserter_->doWork<T>(ep, 0); }
      }
      catch (cet::exception & e) {
        actions::ActionCodes action = (T::isEvent_ ? act_table_->find(e.root_cause()) : actions::Rethrow);
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
      runEndPaths<T>(ep);
    }
    catch (cet::exception & ex) {
      actions::ActionCodes action = (T::isEvent_ ? act_table_->find(ex.root_cause()) : actions::Rethrow);
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
  Schedule::runTriggerPaths(typename T::MyPrincipal & ep)
  {
    using std::placeholders::_1;
    cet::for_all(trig_paths_,
                 std::bind(&Path::processOneOccurrence<T>, _1, std::ref(ep)));
    return results_->accept();
  }

  template <typename T>
  void
  Schedule::runEndPaths(typename T::MyPrincipal & ep)
  {
    using std::placeholders::_1;
    // Note there is no state-checking safety controlling the
    // activation/deactivation of endpaths.
    cet::for_all(end_paths_,
                 std::bind(&Path::processOneOccurrence<T>, _1, std::ref(ep)));
  }

}  // art

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
