// ======================================================================
//
// Class Schedule controls the execution of paths and endpaths.
//
// ======================================================================

#include "art/Framework/Core/Schedule.h"

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/algorithm"
#include "cpp0x/functional"
#include "cpp0x/numeric"
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <list>

using namespace cet;
using namespace fhicl;
using namespace mf;
using namespace std;
using namespace art;

using std::placeholders::_1;
using std::placeholders::_2;

art::Schedule::Schedule(ParameterSet const & proc_pset,
                        art::TriggerNamesService const & tns,
                        WorkerRegistry & wreg,
                        MasterProductRegistry & pregistry,
                        ActionTable & actions,
                        std::shared_ptr<ActivityRegistry> areg):
  process_pset_(proc_pset),
  worker_reg_(&wreg),
  act_table_(&actions),
  processName_(tns.getProcessName()),
  actReg_(areg),
  trig_name_list_(tns.getTrigPaths()),
  end_path_name_list_(tns.getEndPaths()),
  results_(new HLTGlobalStatus(trig_name_list_.size())),
  endpath_results_(), // delay!
  results_inserter_(),
  all_workers_(),
  all_output_workers_(),
  trig_paths_(),
  end_paths_(),
  demand_branches_(),
  wantSummary_(tns.wantSummary()),
  total_events_(),
  total_passed_(),
  stopwatch_(new RunStopwatch::StopwatchPointer::element_type)
{
  assert(actReg_);
  ParameterSet services(process_pset_.get<ParameterSet>("services",
                                                        ParameterSet()));
  ParameterSet opts(services.get<ParameterSet>("scheduler", ParameterSet()));
  bool hasPath = false;
  int trig_bitpos = 0;
  for (auto const & trig_name : trig_name_list_) {
    fillTrigPath(trig_bitpos, trig_name, results_, pregistry);
    ++trig_bitpos;
    hasPath = true;
  }
  if (hasPath) {
    // the results inserter stands alone
    makeTriggerResultsInserter(tns.getTriggerPSet(), pregistry);
    addToAllWorkers(results_inserter_.get());
  }
  TrigResPtr epptr(new HLTGlobalStatus(end_path_name_list_.size()));
  endpath_results_ = epptr;
  // fill normal endpaths
  vstring::iterator eib(end_path_name_list_.begin());
  vstring::iterator eie(end_path_name_list_.end());
  for (int bitpos = 0; eib != eie; ++eib, ++bitpos)
  { fillEndPath(bitpos, *eib, pregistry); }
  //See if all modules were used
  set<string> usedWorkerLabels;
  for (auto const & worker_ptr : all_workers_)
  { usedWorkerLabels.insert(worker_ptr->description().moduleLabel_); }
  vstring const & modulesInConfig(proc_pset.get<vstring >("all_modules",
                                                          vstring()));
  set<string> modulesInConfigSet(modulesInConfig.begin(),
                                 modulesInConfig.end());
  vstring unusedLabels;
  set_difference(modulesInConfigSet.begin(), modulesInConfigSet.end(),
                 usedWorkerLabels.begin(), usedWorkerLabels.end(),
                 back_inserter(unusedLabels));
  //does the configuration say we should allow on demand?
  bool allowUnscheduled = opts.get<bool>("allowUnscheduled", false);
  if (!unusedLabels.empty()) {
    ParameterSet empty;
    ParameterSet physics =   process_pset_.get<ParameterSet>("physics");
    ParameterSet producers = physics.get<ParameterSet>("producers", empty);
    ParameterSet filters   = physics.get<ParameterSet>("filters", empty);
    ParameterSet analyzers = physics.get<ParameterSet>("analyzers", empty);
    ParameterSet outputs   = process_pset_.get<ParameterSet>("outputs", empty);
    //Need to
    // 1) create worker
    // 2) if it is a WorkerT<EDProducer>, add it to our list
    // 3) hand list to our delayed reader
    vstring  shouldBeUsedLabels;
    OnDemandWorkers onDemandWorkers;
    for (vstring::iterator
         itLabel = unusedLabels.begin(),
         itLabelEnd = unusedLabels.end();
         itLabel != itLabelEnd;
         ++itLabel) {
      if (allowUnscheduled) {
        // Need to hold onto the parameters long enough to
        // make the call to getWorker
        ParameterSet workersParams;
        producers.get_if_present(*itLabel, workersParams) ||
        filters.get_if_present(*itLabel, workersParams) ||
        outputs.get_if_present(*itLabel, workersParams) ||
        analyzers.get_if_present(*itLabel, workersParams);
        WorkerParams params(proc_pset, workersParams,
                            pregistry, *act_table_,
                            processName_, getReleaseVersion(),
                            getPassID());
        Worker * newWorker(wreg.getWorker(params));
        if (dynamic_cast<WorkerT<EDProducer>*>(newWorker) ||
            dynamic_cast<WorkerT<EDFilter>*>(newWorker)) {
          onDemandWorkers.
          push_back(cet::exempt_ptr<Worker>(newWorker));
          // add to list so it gets reset each new event
          addToAllWorkers(newWorker);
        }
        else {
          //not a producer so should be marked as not used
          shouldBeUsedLabels.push_back(*itLabel);
        }
      }
      else {
        // everything is marked are unused so no 'on demand'
        // allowed
        shouldBeUsedLabels.push_back(*itLabel);
      }
    }
    if (allowUnscheduled) {
      BranchesByModuleLabel branchLookup;
      fillBranchLookup(pregistry.productList(), branchLookup);
      catalogOnDemandBranches(onDemandWorkers, branchLookup);
    }
    if (!shouldBeUsedLabels.empty()) {
      ostringstream unusedStream;
      unusedStream << "'" << shouldBeUsedLabels.front() << "'";
      for (vstring::iterator
           i = shouldBeUsedLabels.begin() + 1,
           e = shouldBeUsedLabels.end();
           i != e;
           ++i) {
        unusedStream << ",'" << *i << "'";
      }
      LogInfo("path")
          << "The following module labels are not assigned to any path:\n"
          << unusedStream.str()
          << "\n";
    }
  }
  // All the workers should be in all_workers_ by this point. Thus
  // we can now fill all_output_workers_.  We provide a little
  // sanity-check to make sure no code modifications alter the
  // number of workers at a later date... Much better would be to
  // refactor this huge constructor into a series of well-named
  // private functions.
  size_t all_workers_count = all_workers_.size();
  for (auto const & worker_ptr : all_workers_) {
    OutputWorker * ow = dynamic_cast<OutputWorker *>(worker_ptr);
    if (ow) { all_output_workers_.push_back(ow); }
  }
  pregistry.setFrozen();
  // Test path invariants.
  pathConsistencyCheck(all_workers_count);
  ProductMetaData::create_instance(pregistry);
} // art::Schedule::Schedule

bool art::Schedule::terminate() const
{
  if (all_output_workers_.empty()) { return false; }
  for (auto const & ow_ptr : all_output_workers_) {
    if (ow_ptr->limitReached() == false) { return false; }
  }
  LogInfo("SuccessfulTermination")
      << "The job is terminating successfully because each output module\n"
      << "has reached its configured limit.\n";
  return true;
}

void
art::Schedule::fillWorkers(string const & name,
                           PathWorkers & out,
                           bool isTrigPath,
                           MasterProductRegistry & pregistry)
{
  ParameterSet empty;
  ParameterSet physics =   process_pset_.get<ParameterSet>("physics");
  vstring modnames =       physics.get<vstring >(name);
  ParameterSet producers = physics.get<ParameterSet>("producers", empty);
  ParameterSet filters   = physics.get<ParameterSet>("filters", empty);
  ParameterSet analyzers = physics.get<ParameterSet>("analyzers", empty);
  ParameterSet outputs   = process_pset_.get<ParameterSet>("outputs", empty);
  //vstring::iterator it(modnames.begin()),ie(modnames.end());
  PathWorkers tmpworkers;
  for (auto const & modname : modnames) {
    WorkerInPath::FilterAction filterAction = WorkerInPath::Normal;
    if (modname[0] == '!')       { filterAction = WorkerInPath::Veto; }
    else if (modname[0] == '-')  { filterAction = WorkerInPath::Ignore; }
    string realname(modname);
    if (filterAction != WorkerInPath::Normal) { realname.erase(0, 1); }
    ParameterSet modpset;
    // Look for the module's parameter set in the module
    // groups. If we're a trigger path, the search order is:
    // producers, filters, outputs, analyzers; otherwise it is:
    // analyzers, outputs, producers, filters.
    if ((isTrigPath ? producers : analyzers).get_if_present(realname, modpset) ||
        (isTrigPath ? filters : outputs).get_if_present(realname, modpset) ||
        (isTrigPath ? analyzers : producers).get_if_present(realname, modpset) ||
        (isTrigPath ? outputs : filters).get_if_present(realname, modpset)) {
      WorkerParams params(process_pset_, modpset, pregistry, *act_table_,
                          processName_, getReleaseVersion(), getPassID());
      WorkerInPath w(worker_reg_->getWorker(params), filterAction);
      tmpworkers.push_back(w);
    }
    else {
      string pathType("endpath");
      if (!search_all(end_path_name_list_, name)) {
        pathType = string("path");
      }
      throw art::Exception(art::errors::Configuration)
          << "The unknown module label '"
          << realname
          << "' appears in "
          << pathType
          << " '"
          << name
          << "'\nplease check spelling or remove that label from the path.";
    }
  }
  out.swap(tmpworkers);
}

void
art::Schedule::fillTrigPath(int bitpos,
                            string const & name,
                            TrigResPtr trptr,
                            MasterProductRegistry & pregistry)
{
  PathWorkers tmpworkers;
  Workers holder;
  fillWorkers(name, tmpworkers, true, pregistry);
  for (auto const & worker : tmpworkers) { holder.push_back(worker.getWorker()); }
  if (!tmpworkers.empty()) {
    Path p(bitpos, name, tmpworkers, trptr,
           process_pset_, *act_table_, actReg_, false);
    trig_paths_.push_back(p);
  }
  for_all(holder, std::bind(&Schedule::addToAllWorkers, this, _1));
}

void art::Schedule::fillEndPath(int bitpos, string const & name,
                                MasterProductRegistry & pregistry)
{
  PathWorkers tmpworkers;
  fillWorkers(name, tmpworkers, false, pregistry);
  Workers holder;
  for (auto const & pw : tmpworkers) { holder.push_back(pw.getWorker()); }
  if (!tmpworkers.empty()) {
    Path p(bitpos, name, tmpworkers, endpath_results_,
           process_pset_, *act_table_, actReg_, true);
    end_paths_.push_back(p);
  }
  for_all(holder, std::bind(&Schedule::addToAllWorkers, this, _1));
}

void art::Schedule::endJob()
{
  bool failure = false;
  Exception error(errors::EndJobFailure);
  for (Workers::iterator
       i = workersBegin(),
       e = workersEnd();
       i != e; ++i) {
    try {
      (*i)->endJob();
    }
    catch (cet::exception & e) {
      error << "cet::exception caught in Schedule::endJob\n"
            << e.explain_self();
      failure = true;
    }
    catch (std::exception & e) {
      error << "Standard library exception caught in Schedule::endJob\n"
            << e.what();
      failure = true;
    }
    catch (...) {
      error << "Unknown exception caught in Schedule::endJob\n";
      failure = true;
    }
  }
  if (failure) { throw error; }
  writeSummary();
}

void art::Schedule::writeSummary()
{
  Paths::const_iterator pi, pe;
  // The trigger report (pass/fail etc.):
  // Printed even if summary not requested, per issue #1864.
  LogAbsolute("ArtSummary") << "";
  LogAbsolute("ArtSummary") << "TrigReport " <<
                            "---------- Event  Summary ------------";
  LogAbsolute("ArtSummary") << "TrigReport"
                            << " Events total = " << totalEvents()
                            << " passed = " << totalEventsPassed()
                            << " failed = " << (totalEventsFailed())
                            << "";
  if (wantSummary_) {
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " <<
                              "---------- Path   Summary ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    pi = trig_paths_.begin();
    pe = trig_paths_.end();
    for (; pi != pe; ++pi) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5) << 1
                                << right << setw(5) << pi->bitPosition() << " "
                                << right << setw(10) << pi->timesRun() << " "
                                << right << setw(10) << pi->timesPassed() << " "
                                << right << setw(10) << pi->timesFailed() << " "
                                << right << setw(10) << pi->timesExcept() << " "
                                << pi->name() << "";
    }
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " <<
                              "-------End-Path   Summary ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    pi = end_paths_.begin();
    pe = end_paths_.end();
    for (; pi != pe; ++pi) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5) << 0
                                << right << setw(5) << pi->bitPosition() << " "
                                << right << setw(10) << pi->timesRun() << " "
                                << right << setw(10) << pi->timesPassed() << " "
                                << right << setw(10) << pi->timesFailed() << " "
                                << right << setw(10) << pi->timesExcept() << " "
                                << pi->name() << "";
    }
    pi = trig_paths_.begin();
    pe = trig_paths_.end();
    for (; pi != pe; ++pi) {
      LogAbsolute("ArtSummary") << "";
      LogAbsolute("ArtSummary") << "TrigReport " << "---------- Modules in Path: " <<
                                pi->name() << " ------------";
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(10) << "Trig Bit#" << " "
                                << right << setw(10) << "Visited" << " "
                                << right << setw(10) << "Passed" << " "
                                << right << setw(10) << "Failed" << " "
                                << right << setw(10) << "Error" << " "
                                << "Name" << "";
      for (unsigned int i = 0; i < pi->size(); ++i) {
        LogAbsolute("ArtSummary") << "TrigReport "
                                  << right << setw(5) << 1
                                  << right << setw(5) << pi->bitPosition() << " "
                                  << right << setw(10) << pi->timesVisited(i) << " "
                                  << right << setw(10) << pi->timesPassed(i) << " "
                                  << right << setw(10) << pi->timesFailed(i) << " "
                                  << right << setw(10) << pi->timesExcept(i) << " "
                                  << pi->getWorker(i)->description().moduleLabel_ << "";
      }
    }
  }
  // Printed even if summary not requested, per issue #1864.
  pi = end_paths_.begin();
  pe = end_paths_.end();
  for (; pi != pe; ++pi) {
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " << "------ Modules in End-Path: " <<
                              pi->name() << " ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Visited" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (unsigned int i = 0; i < pi->size(); ++i) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5) << 0
                                << right << setw(5) << pi->bitPosition() << " "
                                << right << setw(10) << pi->timesVisited(i) << " "
                                << right << setw(10) << pi->timesPassed(i) << " "
                                << right << setw(10) << pi->timesFailed(i) << " "
                                << right << setw(10) << pi->timesExcept(i) << " "
                                << pi->getWorker(i)->description().moduleLabel_ << "";
    }
  }
  Workers::iterator ai, ae;
  if (wantSummary_) {
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " <<
                              "---------- Module Summary ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Visited" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (ai = workersBegin(), ae = workersEnd(); ai != ae; ++ai) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(10) << (*ai)->timesVisited() << " "
                                << right << setw(10) << (*ai)->timesRun() << " "
                                << right << setw(10) << (*ai)->timesPassed() << " "
                                << right << setw(10) << (*ai)->timesFailed() << " "
                                << right << setw(10) << (*ai)->timesExcept() << " "
                                << (*ai)->description().moduleLabel_ << "";
    }
  }
  LogAbsolute("ArtSummary") << "";
  // The timing report (CPU and Real Time):
  LogAbsolute("ArtSummary") << "TimeReport " <<
                            "---------- Time  Summary ---[sec]----";
  LogAbsolute("ArtSummary") << "TimeReport"
                            << setprecision(6) << fixed
                            << " CPU = " << timeCpuReal().first
                            << " Real = " << timeCpuReal().second
                            << "";
  LogAbsolute("ArtSummary") << "";
  if (wantSummary_) {
    LogAbsolute("ArtSummary") << "TimeReport " <<
                              "---------- Event  Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport"
                              << setprecision(6) << fixed
                              << " CPU/event = " << timeCpuReal().first / max(1, totalEvents())
                              << " Real/event = " << timeCpuReal().second / max(1, totalEvents())
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TimeReport " <<
                              "---------- Path   Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per path-run "
                              << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    pi = trig_paths_.begin();
    pe = trig_paths_.end();
    for (; pi != pe; ++pi) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << pi->timeCpuReal().first / max(1, totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().second / max(1, totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().first / max(1, pi->timesRun()) << " "
                                << right << setw(10) << pi->timeCpuReal().second / max(1, pi->timesRun()) << " "
                                << pi->name() << "";
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per path-run "
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TimeReport " <<
                              "-------End-Path   Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per endpath-run "
                              << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    pi = end_paths_.begin();
    pe = end_paths_.end();
    for (; pi != pe; ++pi) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << pi->timeCpuReal().first / max(1, totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().second / max(1, totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().first / max(1, pi->timesRun()) << " "
                                << right << setw(10) << pi->timeCpuReal().second / max(1, pi->timesRun()) << " "
                                << pi->name() << "";
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per endpath-run "
                              << "";
    pi = trig_paths_.begin();
    pe = trig_paths_.end();
    for (; pi != pe; ++pi) {
      LogAbsolute("ArtSummary") << "";
      LogAbsolute("ArtSummary") << "TimeReport " << "---------- Modules in Path: " <<
                                pi->name() << " ---[sec]----";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(22) << "per event "
                                << right << setw(22) << "per module-visit "
                                << "";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << "Name" << "";
      for (unsigned int i = 0; i < pi->size(); ++i) {
        LogAbsolute("ArtSummary") << "TimeReport "
                                  << setprecision(6) << fixed
                                  << right << setw(10) << pi->timeCpuReal(i).first / max(1, totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second / max(1, totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).first / max(1,
                                      pi->timesVisited(i)) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second / max(1,
                                      pi->timesVisited(i)) << " "
                                  << pi->getWorker(i)->description().moduleLabel_ << "";
      }
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-visit "
                              << "";
    pi = end_paths_.begin();
    pe = end_paths_.end();
    for (; pi != pe; ++pi) {
      LogAbsolute("ArtSummary") << "";
      LogAbsolute("ArtSummary") << "TimeReport " << "------ Modules in End-Path: " <<
                                pi->name() << " ---[sec]----";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(22) << "per event "
                                << right << setw(22) << "per module-visit "
                                << "";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << "Name" << "";
      for (unsigned int i = 0; i < pi->size(); ++i) {
        LogAbsolute("ArtSummary") << "TimeReport "
                                  << setprecision(6) << fixed
                                  << right << setw(10) << pi->timeCpuReal(i).first / max(1, totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second / max(1, totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).first / max(1,
                                      pi->timesVisited(i)) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second / max(1,
                                      pi->timesVisited(i)) << " "
                                  << pi->getWorker(i)->description().moduleLabel_ << "";
      }
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-visit "
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TimeReport " <<
                              "---------- Module Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-run "
                              << right << setw(22) << "per module-visit "
                              << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    ai = workersBegin();
    ae = workersEnd();
    for (; ai != ae; ++ai) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << (*ai)->timeCpuReal().first / max(1,
                                    totalEvents()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().second / max(1,
                                    totalEvents()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().first / max(1,
                                    (*ai)->timesRun()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().second / max(1,
                                    (*ai)->timesRun()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().first / max(1,
                                    (*ai)->timesVisited()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().second / max(1,
                                    (*ai)->timesVisited()) << " "
                                << (*ai)->description().moduleLabel_ << "";
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-run "
                              << right << setw(22) << "per module-visit "
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "T---Report end!" << "";
    LogAbsolute("ArtSummary") << "";
  }
}

void art::Schedule::closeOutputFiles()
{
  for_all(all_output_workers_, std::bind(&OutputWorker::closeFile, _1));
}

void art::Schedule::openOutputFiles(FileBlock & fb)
{
  for_all(all_output_workers_, std::bind(&OutputWorker::openFile, _1,
                                         std::cref(fb)));
}

void art::Schedule::writeRun(RunPrincipal const & rp)
{
  for_all(all_output_workers_, std::bind(&OutputWorker::writeRun, _1,
                                         std::cref(rp)));
}

void art::Schedule::writeSubRun(SubRunPrincipal const & srp)
{
  for_all(all_output_workers_, std::bind(&OutputWorker::writeSubRun, _1,
                                         std::cref(srp)));
}

bool art::Schedule::shouldWeCloseOutput() const
{
  // Return true iff at least one output module returns true.
  return (find_if(all_output_workers_.begin(), all_output_workers_.end(),
                  std::bind(&OutputWorker::shouldWeCloseFile, _1))
          != all_output_workers_.end());
}

void art::Schedule::respondToOpenInputFile(FileBlock const & fb)
{
  for_all(all_workers_, std::bind(&Worker::respondToOpenInputFile, _1,
                                  std::cref(fb)));
}

void art::Schedule::respondToCloseInputFile(FileBlock const & fb)
{
  for_all(all_workers_, std::bind(&Worker::respondToCloseInputFile, _1,
                                  std::cref(fb)));
}

void art::Schedule::respondToOpenOutputFiles(FileBlock const & fb)
{
  for_all(all_workers_, std::bind(&Worker::respondToOpenOutputFiles, _1,
                                  std::cref(fb)));
}

void art::Schedule::respondToCloseOutputFiles(FileBlock const & fb)
{
  for_all(all_workers_, std::bind(&Worker::respondToCloseOutputFiles, _1,
                                  std::cref(fb)));
}

void art::Schedule::beginJob()
{
  for_all(all_workers_, std::bind(&Worker::beginJob, _1));
}

void
art::Schedule::clearCounters()
{
  total_events_ = total_passed_ = 0;
  for_all(trig_paths_, std::bind(&Path::clearCounters, _1));
  for_all(end_paths_, std::bind(&Path::clearCounters, _1));
  for_all(all_workers_, std::bind(&Worker::clearCounters, _1));
}

void
art::Schedule::getAllWorkers(std::vector<Worker *> & out)
{
  copy_all(all_workers_, std::back_inserter(out));
}

void
art::Schedule::resetAll()
{
  for_all(all_workers_, std::bind(&Worker::reset, _1));
  results_->reset();
  endpath_results_->reset();
}

void
art::Schedule::addToAllWorkers(Worker * w)
{
  if (!search_all(all_workers_, w)) { all_workers_.push_back(w); }
}

// FIXME: This can work with generic Principals just as soon as the
// metadata can handle (or obviate) a BranchID <-> ProductID
// conversion for all principal types.
void
art::Schedule::setupOnDemandSystem(EventPrincipal & p)
{
  BranchType b(p.branchType());
  for (OnDemandBranches::const_iterator
       itBranch = demand_branches_.begin(),
       itBranchEnd = demand_branches_.end();
       itBranch != itBranchEnd;
       ++itBranch) {
    if (itBranch->second->branchType() == b) {
      p.addOnDemandGroup(*itBranch->second, itBranch->first);
    }
  }
}

void
art::Schedule::makeTriggerResultsInserter(ParameterSet const & trig_pset,
                                          MasterProductRegistry & pregistry)
{
  WorkerParams work_args(process_pset_, trig_pset, pregistry, *act_table_,
                         processName_);
  ModuleDescription md;
  md.parameterSetID_ = trig_pset.id();
  md.moduleName_ = "TriggerResultInserter";
  md.moduleLabel_ = "TriggerResults";
  md.processConfiguration_ = ProcessConfiguration(processName_,
                                                  process_pset_.id(), getReleaseVersion(), getPassID());
  actReg_->sPreModuleConstruction.invoke(md);
  unique_ptr<EDProducer> producer(new TriggerResultInserter(trig_pset, results_));
  actReg_->sPostModuleConstruction.invoke(md);
  results_inserter_.reset(new WorkerT<EDProducer>(std::move(producer), md,
                                                  work_args));
  results_inserter_->setActivityRegistry(actReg_);
}

void art::Schedule::fillBranchLookup(ProductList const & pList,
                                     BranchesByModuleLabel & branchLookup) const
{
  for (ProductList::const_iterator
       i = pList.begin(),
       e = pList.end();
       i != e;
       ++i) {
    branchLookup.insert(std::make_pair(i->second.moduleLabel(),
                                       cet::exempt_ptr<BranchDescription const>(&i->second)));
  }
}

void art::Schedule::catalogOnDemandBranches(OnDemandWorkers const & odw,
                                            BranchesByModuleLabel const & branchLookup)
{
  cet::for_all(odw,
               std::bind
               (&art::Schedule::catalogOneOnDemandWorker,
                this,
                std::placeholders::_1,
                branchLookup));
}

void art::Schedule::catalogOneOnDemandWorker(cet::exempt_ptr<Worker> wp,
                                             BranchesByModuleLabel const & branchLookup)
{
  BranchesByModuleLabel::const_iterator
  lb(branchLookup.lower_bound(wp->label())),
  e(branchLookup.end()),
  ub(branchLookup.upper_bound(wp->label()));
  if (lb == ub) { return; } // This worker produces nothing.
  for (BranchesByModuleLabel::const_iterator i = lb;
       i != ub;
       ++i) {
    demand_branches_.insert(std::make_pair(wp, i->second));
  }
}

void art::Schedule::pathConsistencyCheck(size_t expected_num_workers
                                         __attribute__((
                                                         unused))) const
{
  // Major sanity check: make sure nobody has added a worker after
  // we've already relied on all_workers_ being full. Failure here
  // indicates a logic error in Schedule().
  assert(expected_num_workers == all_workers_.size() &&
         "INTERNAL ASSERTION ERROR: all_workers_ changed after being used.");
  size_t numFailures = 0;
  numFailures = std::accumulate(trig_paths_.begin(),
                                trig_paths_.end(),
                                numFailures,
                                std::bind(&Schedule::accumulateConsistencyFailures,
                                          this,
                                          _1,
                                          _2,
                                          false));
  numFailures = std::accumulate(end_paths_.begin(),
                                end_paths_.end(),
                                numFailures,
                                std::bind(&Schedule::accumulateConsistencyFailures,
                                          this,
                                          _1,
                                          _2,
                                          true));
  if (numFailures > 0) {
    // TODO: Throw correct exception.
    throw cet::exception("IllegalPathEntries")
        << "Found a total of "
        << numFailures
        << " illegal entries in paths; see error log for full list.";
  }
}

size_t art::Schedule::accumulateConsistencyFailures(size_t current_num_failures,
                                                    art::Path const & path,
                                                    bool isEndPath) const
{
  return current_num_failures +
         checkOnePath(path, isEndPath);
}

size_t art::Schedule::checkOnePath(Path const & path, bool isEndPath) const
{
  std::vector<std::string> results;
  std::ostringstream message;
  if (isEndPath) {
    message << "The following modules are illegal in an end path  (\""
            << path.name()
            << "\"): they modify the event "
            << "and should be in a standard (trigger) path.";
    path.findEventModifiers(results);
  }
  else {
    message << "The following modules are illegal in a standard (trigger) path (\""
            << path.name()
            << "\"): they are observers "
            << "and should be in an end path.";
    path.findEventObservers(results);
  }
  size_t nFailures = results.size();
  if (nFailures > 0) {
    message << "\n";
    cet::copy_all(results, std::ostream_iterator<std::string>(message, "\n"));
    LogError("IllegalPathEntries")
        << message.str();
  }
  return results.size();
}
