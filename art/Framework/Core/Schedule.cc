// ======================================================================
//
// Schedule
//
// ======================================================================

#include "art/Framework/Core/Schedule.h"

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "boost/bind.hpp"
#include "boost/ref.hpp"
#include "cpp0x/functional"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <list>
#include <numeric>

using namespace cet;
using namespace fhicl;
using namespace mf;
using namespace std;
using namespace art;

using std::placeholders::_1;
using std::placeholders::_2;

namespace {

   // Function template to transform each element in the input range to
   // a value placed into the output range. The supplied function
   // should take a const_reference to the 'input', and write to a
   // reference to the 'output'.
   template <class InputIterator, class ForwardIterator, class Func>
   void
   transform_into(InputIterator begin, InputIterator end,
                  ForwardIterator out, Func func) {
      for (; begin != end; ++begin, ++out) func(*begin, *out);
   }

   // Function template that takes a sequence 'from', a sequence
   // 'to', and a callable object 'func'. It and applies
   // transform_into to fill the 'to' sequence with the values
   // calcuated by the callable object, taking care to fill the
   // outupt only if all calls succeed.
   template <class FROM, class TO, class FUNC>
   void
   fill_summary(FROM const& from, TO& to, FUNC func) {
      TO temp(from.size());
      transform_into(from.begin(), from.end(), temp.begin(), func);
      to.swap(temp);
   }

   // -----------------------------

}  // namespace

  // -----------------------------

namespace art {

  // -----------------------------

  Schedule::Schedule(ParameterSet const& proc_pset,
                     art::service::TriggerNamesService& tns,
                     WorkerRegistry& wreg,
                     ProductRegistry& preg,
                     ActionTable& actions,
                     boost::shared_ptr<ActivityRegistry> areg):
    pset_(proc_pset),
    worker_reg_(&wreg),
    prod_reg_(&preg),
    act_table_(&actions),
    processName_(tns.getProcessName()),
    actReg_(areg),
    state_(Ready),
    trig_name_list_(tns.getTrigPaths()),
    end_path_name_list_(tns.getEndPaths()),
    results_        (new HLTGlobalStatus(trig_name_list_.size())),
    endpath_results_(), // delay!
    results_inserter_(),
    all_workers_(),
    all_output_workers_(),
    trig_paths_(),
    end_paths_(),
    wantSummary_(tns.wantSummary()),
    total_events_(),
    total_passed_(),
    stopwatch_(new RunStopwatch::StopwatchPointer::element_type),
    unscheduled_(new UnscheduledCallProducer),
    demandBranches_(),
    endpathsAreActive_(true)
  {
    ParameterSet services(pset_.get<ParameterSet>("services", ParameterSet()));
    ParameterSet opts(services.get<ParameterSet>("scheduler", ParameterSet()));
    bool hasPath = false;

    int trig_bitpos = 0;
    for (vstring::const_iterator i = trig_name_list_.begin(),
           e = trig_name_list_.end();
         i != e;
         ++i) {
      fillTrigPath(trig_bitpos,*i, results_);
      ++trig_bitpos;
      hasPath = true;
    }

    if (hasPath) {
      // the results inserter stands alone
      results_inserter_ = makeInserter(tns.getTriggerPSet());
      addToAllWorkers(results_inserter_.get());
    }

    TrigResPtr epptr(new HLTGlobalStatus(end_path_name_list_.size()));
    endpath_results_ = epptr;

    // fill normal endpaths
    vstring::iterator eib(end_path_name_list_.begin()),eie(end_path_name_list_.end());
    for(int bitpos = 0; eib != eie; ++eib, ++bitpos) {
      fillEndPath(bitpos, *eib);
    }

    //See if all modules were used
    set<string> usedWorkerLabels;
    for(Workers::iterator itWorker=workersBegin();
        itWorker != workersEnd();
        ++itWorker) {
      usedWorkerLabels.insert((*itWorker)->description().moduleLabel_);
    }
    vector<string> modulesInConfig(proc_pset.get<vector<string> >("all_modules", vector<string>()));
    set<string> modulesInConfigSet(modulesInConfig.begin(),modulesInConfig.end());
    vector<string> unusedLabels;
    set_difference(modulesInConfigSet.begin(),modulesInConfigSet.end(),
                   usedWorkerLabels.begin(),usedWorkerLabels.end(),
                   back_inserter(unusedLabels));
    //does the configuration say we should allow on demand?
    bool allowUnscheduled = opts.get<bool>("allowUnscheduled", false);
    set<string> unscheduledLabels;
    if(!unusedLabels.empty()) {
      //Need to
      // 1) create worker
      // 2) if it is a WorkerT<EDProducer>, add it to our list
      // 3) hand list to our delayed reader
      vector<string>  shouldBeUsedLabels;

      for(vector<string>::iterator itLabel = unusedLabels.begin(), itLabelEnd = unusedLabels.end();
          itLabel != itLabelEnd;
          ++itLabel) {
        if (allowUnscheduled) {
          //Need to hold onto the parameters long enough to make the call to getWorker
          ParameterSet workersParams(proc_pset.get<ParameterSet>(*itLabel));
          WorkerParams params(proc_pset, workersParams,
                              *prod_reg_, *act_table_,
                              processName_, getReleaseVersion(), getPassID());
          Worker* newWorker(wreg.getWorker(params));
          if (dynamic_cast<WorkerT<EDProducer>*>(newWorker) ||
              dynamic_cast<WorkerT<EDFilter>*>(newWorker) ) {
            unscheduledLabels.insert(*itLabel);
            unscheduled_->addWorker(newWorker);
            //add to list so it gets reset each new event
            addToAllWorkers(newWorker);
          } else {
            //not a producer so should be marked as not used
            shouldBeUsedLabels.push_back(*itLabel);
          }
        } else {
          //everthing is marked are unused so no 'on demand' allowed
          shouldBeUsedLabels.push_back(*itLabel);
        }
      }
      if(!shouldBeUsedLabels.empty()) {
        ostringstream unusedStream;
        unusedStream << "'"<< shouldBeUsedLabels.front() <<"'";
        for(vector<string>::iterator itLabel = shouldBeUsedLabels.begin() + 1,
              itLabelEnd = shouldBeUsedLabels.end();
            itLabel != itLabelEnd;
            ++itLabel) {
          unusedStream <<",'" << *itLabel<<"'";
        }
        LogInfo("path")
          << "The following module labels are not assigned to any path:\n"
          <<unusedStream.str()
          <<"\n";
      }
    }

    // All the workers should be in all_workers_ by this point. Thus
    // we can now fill all_output_workers_.  We provide a little
    // sanity-check to make sure no code modifications alter the
    // number of workers at a later date... Much better would be to
    // refactor this huge constructor into a series of well-named
    // private functions.
    size_t all_workers_count = all_workers_.size();
    for (Workers::iterator i = all_workers_.begin(), e = all_workers_.end();
         i != e;
         ++i)   {
      OutputWorker* ow = dynamic_cast<OutputWorker*>(*i);
      if (ow) all_output_workers_.push_back(ow);
    }

    // Now that the output workers are filled in, set any output limits.
    limitOutput();

    prod_reg_->setFrozen();

    if (allowUnscheduled) {
       // Now that these have been set, we can create the list of
       // Branches we need for the 'on demand.'
       catalogOnDemandBranches(unscheduledLabels);
    }

    // Test path invariants.
    pathConsistencyCheck(all_workers_count);

  } // Schedule::Schedule

  void
  Schedule::limitOutput() {
  #if 0
    string const output("output");

    ParameterSet maxEventsPSet(pset_.get<ParameterSet>("maxEvents", ParameterSet()));
    int maxEventSpecs = 0;
    int maxEventsOut = -1;
    ParameterSet vMaxEventsOut;
    vector<string> intNamesE = maxEventsPSet.getParameterNamesForType<int>(false);
    if (search_all(intNamesE, output)) {
      maxEventsOut = maxEventsPSet.get<int>(output);
      ++maxEventSpecs;
    }
    vector<string> psetNamesE;
    maxEventsPSet.get<ParameterSet>NameList(psetNamesE, false);
    if (search_all(psetNamesE, output)) {
      vMaxEventsOut = maxEventsPSet.get<ParameterSet>(output);
      ++maxEventSpecs;
    }

    if (maxEventSpecs > 1) {
      throw art::Exception(art::errors::Configuration) <<
        "\nAt most one form of 'output' may appear in the 'maxEvents' parameter set";
    }

    if (maxEventSpecs == 0) {
      return;
    }

    for (OutputWorkers::const_iterator it = all_output_workers_.begin(), itEnd = all_output_workers_.end();
        it != itEnd; ++it) {
      OutputModuleDescription desc(maxEventsOut);
      if (!vMaxEventsOut.empty()) {
        string moduleLabel = (*it)->description().moduleLabel_;
        if (!vMaxEventsOut.empty()) {
          try {
            desc.maxEvents_ = vMaxEventsOut.get<int>(moduleLabel);
          } catch (art::Exception) {
            throw art::Exception(art::errors::Configuration) <<
              "\nNo entry in 'maxEvents' for output module label '" << moduleLabel << "'.\n";
          }
        }
      }
      (*it)->configure(desc);
    }
  #endif
  }

  bool const Schedule::terminate() const {
    if (all_output_workers_.empty()) {
      return false;
    }
    for (OutputWorkers::const_iterator it = all_output_workers_.begin(),
         itEnd = all_output_workers_.end();
         it != itEnd; ++it) {
      if (!(*it)->limitReached()) {
        // Found an output module that has not reached output event count.
        return false;
      }
    }
    LogInfo("SuccessfulTermination")
      << "The job is terminating successfully because each output module\n"
      << "has reached its configured limit.\n";
    return true;
  }

   void Schedule::fillWorkers(string const& name, PathWorkers& out, bool isTrigPath) {
    ParameterSet physics = pset_.get<ParameterSet>("physics");
    vstring modnames = physics.get<vector<string> >(name);
    ParameterSet producers = physics.get<ParameterSet>("producers", ParameterSet());
    ParameterSet filters = physics.get<ParameterSet>("filters", ParameterSet());
    ParameterSet analyzers = physics.get<ParameterSet>("analyzers", ParameterSet());
    ParameterSet outputs = pset_.get<ParameterSet>("outputs", ParameterSet());

    vstring::iterator it(modnames.begin()),ie(modnames.end());
    PathWorkers tmpworkers;

    for(; it != ie; ++it) {

      WorkerInPath::FilterAction filterAction = WorkerInPath::Normal;
      if ((*it)[0] == '!')       filterAction = WorkerInPath::Veto;
      else if ((*it)[0] == '-')  filterAction = WorkerInPath::Ignore;

      string realname = *it;
      if (filterAction != WorkerInPath::Normal) realname.erase(0,1);

      ParameterSet modpset;
      try {
         modpset = (isTrigPath?producers:analyzers).get<ParameterSet>(realname);
      } catch(cet::exception&) {
         try {
            modpset = (isTrigPath?filters:outputs).get<ParameterSet>(realname);
         }
         catch(cet::exception&) {
            try {
               modpset = (isTrigPath?analyzers:producers).get<ParameterSet>(realname);
            }
            catch(cet::exception&) {
               try {
                  modpset = (isTrigPath?outputs:filters).get<ParameterSet>(realname);
               }
               catch(cet::exception&) {
                  string pathType("endpath");
                  if(!search_all(end_path_name_list_, name)) {
                     pathType = string("path");
                  }
                  throw art::Exception(art::errors::Configuration) <<
                     "The unknown module label \"" << realname <<
                     "\" appears in " << pathType << " \"" << name <<
                     "\"\n please check spelling or remove that label from the path.";
               }
            }
         }
      }
      WorkerParams params(pset_, modpset, *prod_reg_, *act_table_,
                          processName_, getReleaseVersion(), getPassID());
      WorkerInPath w(worker_reg_->getWorker(params), filterAction);
      tmpworkers.push_back(w);
    }

    out.swap(tmpworkers);
  }

  void Schedule::fillTrigPath(int bitpos, string const& name, TrigResPtr trptr) {
    PathWorkers tmpworkers;
    Workers holder;
    fillWorkers(name,tmpworkers, true);

    for(PathWorkers::iterator wi(tmpworkers.begin()),
          we(tmpworkers.end()); wi != we; ++wi) {
      holder.push_back(wi->getWorker());
    }

    // an empty path will cause an extra bit that is not used
    if(!tmpworkers.empty()) {
      Path p(bitpos,name,tmpworkers,trptr,pset_,*act_table_,actReg_,false);
      trig_paths_.push_back(p);
    }
    for_all(holder, boost::bind(&art::Schedule::addToAllWorkers, this, _1));
  }

  void Schedule::fillEndPath(int bitpos, string const& name) {
    PathWorkers tmpworkers;
    fillWorkers(name,tmpworkers, false);
    Workers holder;

    for(PathWorkers::iterator wi(tmpworkers.begin()),
          we(tmpworkers.end()); wi != we; ++wi) {
      holder.push_back(wi->getWorker());
    }

    if (!tmpworkers.empty()) {
      Path p(bitpos,name,tmpworkers,endpath_results_,pset_,*act_table_,actReg_,true);
      end_paths_.push_back(p);
    }
    for_all(holder, boost::bind(&art::Schedule::addToAllWorkers, this, _1));
  }

  void Schedule::endJob() {
    bool failure = false;
    cet::exception accumulated("endJob");
    Workers::iterator ai(workersBegin()),ae(workersEnd());
    for(; ai != ae; ++ai) {
      try {
        (*ai)->endJob();
      }
      catch (cet::exception& e) {
        accumulated << "cet::exception caught in Schedule::endJob\n"
                    << e.explain_self();
        failure = true;
      }
      catch (std::exception& e) {
        accumulated << "Standard library exception caught in Schedule::endJob\n"
                    << e.what();
        failure = true;
      }
      catch (...) {
        accumulated << "Unknown exception caught in Schedule::endJob\n";
        failure = true;
      }
    }
    if (failure) {
      throw accumulated;
    }


    if(wantSummary_ == false) return;

    Paths::const_iterator pi,pe;

    // The trigger report (pass/fail etc.):

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "TrigReport " << "---------- Event  Summary ------------";
    LogVerbatim("ArtSummary") << "TrigReport"
                              << " Events total = " << totalEvents()
                              << " passed = " << totalEventsPassed()
                              << " failed = " << (totalEventsFailed())
                              << "";

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "TrigReport " << "---------- Path   Summary ------------";
    LogVerbatim("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    pi=trig_paths_.begin();
    pe=trig_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "TrigReport "
                                << right << setw( 5) << 1
                                << right << setw( 5) << pi->bitPosition() << " "
                                << right << setw(10) << pi->timesRun() << " "
                                << right << setw(10) << pi->timesPassed() << " "
                                << right << setw(10) << pi->timesFailed() << " "
                                << right << setw(10) << pi->timesExcept() << " "
                                << pi->name() << "";
    }

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "TrigReport " << "-------End-Path   Summary ------------";
    LogVerbatim("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    pi=end_paths_.begin();
    pe=end_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "TrigReport "
                                << right << setw( 5) << 0
                                << right << setw( 5) << pi->bitPosition() << " "
                                << right << setw(10) << pi->timesRun() << " "
                                << right << setw(10) << pi->timesPassed() << " "
                                << right << setw(10) << pi->timesFailed() << " "
                                << right << setw(10) << pi->timesExcept() << " "
                                << pi->name() << "";
    }

    pi=trig_paths_.begin();
    pe=trig_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "";
      LogVerbatim("ArtSummary") << "TrigReport " << "---------- Modules in Path: " << pi->name() << " ------------";
      LogVerbatim("ArtSummary") << "TrigReport "
                                << right << setw(10) << "Trig Bit#" << " "
                                << right << setw(10) << "Visited" << " "
                                << right << setw(10) << "Passed" << " "
                                << right << setw(10) << "Failed" << " "
                                << right << setw(10) << "Error" << " "
                                << "Name" << "";

      for (unsigned int i = 0; i < pi->size(); ++i) {
        LogVerbatim("ArtSummary") << "TrigReport "
                                  << right << setw( 5) << 1
                                  << right << setw( 5) << pi->bitPosition() << " "
                                  << right << setw(10) << pi->timesVisited(i) << " "
                                  << right << setw(10) << pi->timesPassed(i) << " "
                                  << right << setw(10) << pi->timesFailed(i) << " "
                                  << right << setw(10) << pi->timesExcept(i) << " "
                                  << pi->getWorker(i)->description().moduleLabel_ << "";
      }
    }

    pi=end_paths_.begin();
    pe=end_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "";
      LogVerbatim("ArtSummary") << "TrigReport " << "------ Modules in End-Path: " << pi->name() << " ------------";
      LogVerbatim("ArtSummary") << "TrigReport "
                                << right << setw(10) << "Trig Bit#" << " "
                                << right << setw(10) << "Visited" << " "
                                << right << setw(10) << "Passed" << " "
                                << right << setw(10) << "Failed" << " "
                                << right << setw(10) << "Error" << " "
                                << "Name" << "";

      for (unsigned int i = 0; i < pi->size(); ++i) {
        LogVerbatim("ArtSummary") << "TrigReport "
                                  << right << setw( 5) << 0
                                  << right << setw( 5) << pi->bitPosition() << " "
                                  << right << setw(10) << pi->timesVisited(i) << " "
                                  << right << setw(10) << pi->timesPassed(i) << " "
                                  << right << setw(10) << pi->timesFailed(i) << " "
                                  << right << setw(10) << pi->timesExcept(i) << " "
                                  << pi->getWorker(i)->description().moduleLabel_ << "";
      }
    }

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "TrigReport " << "---------- Module Summary ------------";
    LogVerbatim("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Visited" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    ai=workersBegin();
    ae=workersEnd();
    for(; ai != ae; ++ai) {
      LogVerbatim("ArtSummary") << "TrigReport "
                                << right << setw(10) << (*ai)->timesVisited() << " "
                                << right << setw(10) << (*ai)->timesRun() << " "
                                << right << setw(10) << (*ai)->timesPassed() << " "
                                << right << setw(10) << (*ai)->timesFailed() << " "
                                << right << setw(10) << (*ai)->timesExcept() << " "
                                << (*ai)->description().moduleLabel_ << "";

    }
    LogVerbatim("ArtSummary") << "";

    // The timing report (CPU and Real Time):

    LogVerbatim("ArtSummary") << "TimeReport " << "---------- Event  Summary ---[sec]----";
    LogVerbatim("ArtSummary") << "TimeReport"
                              << setprecision(6) << fixed
                              << " CPU/event = " << timeCpuReal().first/max(1,totalEvents())
                              << " Real/event = " << timeCpuReal().second/max(1,totalEvents())
                              << "";

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "TimeReport " << "---------- Path   Summary ---[sec]----";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per path-run "
                              << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    pi=trig_paths_.begin();
    pe=trig_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << pi->timeCpuReal().first/max(1,totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().second/max(1,totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().first/max(1,pi->timesRun()) << " "
                                << right << setw(10) << pi->timeCpuReal().second/max(1,pi->timesRun()) << " "
                                << pi->name() << "";
    }
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per path-run "
                              << "";

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "TimeReport " << "-------End-Path   Summary ---[sec]----";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per endpath-run "
                              << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    pi=end_paths_.begin();
    pe=end_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << pi->timeCpuReal().first/max(1,totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().second/max(1,totalEvents()) << " "
                                << right << setw(10) << pi->timeCpuReal().first/max(1,pi->timesRun()) << " "
                                << right << setw(10) << pi->timeCpuReal().second/max(1,pi->timesRun()) << " "
                                << pi->name() << "";
    }
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per endpath-run "
                              << "";

    pi=trig_paths_.begin();
    pe=trig_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "";
      LogVerbatim("ArtSummary") << "TimeReport " << "---------- Modules in Path: " << pi->name() << " ---[sec]----";
      LogVerbatim("ArtSummary") << "TimeReport "
                                << right << setw(22) << "per event "
                                << right << setw(22) << "per module-visit "
                                << "";
      LogVerbatim("ArtSummary") << "TimeReport "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << "Name" << "";
      for (unsigned int i = 0; i < pi->size(); ++i) {
        LogVerbatim("ArtSummary") << "TimeReport "
                                  << setprecision(6) << fixed
                                  << right << setw(10) << pi->timeCpuReal(i).first/max(1,totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second/max(1,totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).first/max(1,pi->timesVisited(i)) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second/max(1,pi->timesVisited(i)) << " "
                                  << pi->getWorker(i)->description().moduleLabel_ << "";
      }
    }
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-visit "
                              << "";

    pi=end_paths_.begin();
    pe=end_paths_.end();
    for(; pi != pe; ++pi) {
      LogVerbatim("ArtSummary") << "";
      LogVerbatim("ArtSummary") << "TimeReport " << "------ Modules in End-Path: " << pi->name() << " ---[sec]----";
      LogVerbatim("ArtSummary") << "TimeReport "
                                << right << setw(22) << "per event "
                                << right << setw(22) << "per module-visit "
                                << "";
      LogVerbatim("ArtSummary") << "TimeReport "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << "Name" << "";
      for (unsigned int i = 0; i < pi->size(); ++i) {
        LogVerbatim("ArtSummary") << "TimeReport "
                                  << setprecision(6) << fixed
                                  << right << setw(10) << pi->timeCpuReal(i).first/max(1,totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second/max(1,totalEvents()) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).first/max(1,pi->timesVisited(i)) << " "
                                  << right << setw(10) << pi->timeCpuReal(i).second/max(1,pi->timesVisited(i)) << " "
                                  << pi->getWorker(i)->description().moduleLabel_ << "";
      }
    }
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-visit "
                              << "";

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "TimeReport " << "---------- Module Summary ---[sec]----";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-run "
                              << right << setw(22) << "per module-visit "
                              << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    ai=workersBegin();
    ae=workersEnd();
    for(; ai != ae; ++ai) {
      LogVerbatim("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << (*ai)->timeCpuReal().first/max(1,totalEvents()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().second/max(1,totalEvents()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().first/max(1,(*ai)->timesRun()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().second/max(1,(*ai)->timesRun()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().first/max(1,(*ai)->timesVisited()) << " "
                                << right << setw(10) << (*ai)->timeCpuReal().second/max(1,(*ai)->timesVisited()) << " "
                                << (*ai)->description().moduleLabel_ << "";
    }
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogVerbatim("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-run "
                              << right << setw(22) << "per module-visit "
                              << "";

    LogVerbatim("ArtSummary") << "";
    LogVerbatim("ArtSummary") << "T---Report end!" << "";
    LogVerbatim("ArtSummary") << "";
  }

  void Schedule::closeOutputFiles() {
    for_all(all_output_workers_, boost::bind(&OutputWorker::closeFile, _1));
  }

  void Schedule::openNewOutputFilesIfNeeded() {
    for_all(all_output_workers_, boost::bind(&OutputWorker::openNewFileIfNeeded, _1));
  }

  void Schedule::openOutputFiles(FileBlock & fb) {
    for_all(all_output_workers_, boost::bind(&OutputWorker::openFile, _1, boost::cref(fb)));
  }

  void Schedule::writeRun(RunPrincipal const& rp) {
    for_all(all_output_workers_, boost::bind(&OutputWorker::writeRun, _1, boost::cref(rp)));
  }

  void Schedule::writeSubRun(SubRunPrincipal const& lbp) {
    for_all(all_output_workers_, boost::bind(&OutputWorker::writeSubRun, _1, boost::cref(lbp)));
  }

  bool Schedule::shouldWeCloseOutput() const {
    // Return true iff at least one output module returns true.
    return (find_if(all_output_workers_.begin(), all_output_workers_.end(),
                     boost::bind(&OutputWorker::shouldWeCloseFile, _1))
                     != all_output_workers_.end());
  }

  void Schedule::respondToOpenInputFile(FileBlock const& fb) {
    for_all(all_workers_, boost::bind(&Worker::respondToOpenInputFile, _1, boost::cref(fb)));
  }

  void Schedule::respondToCloseInputFile(FileBlock const& fb) {
    for_all(all_workers_, boost::bind(&Worker::respondToCloseInputFile, _1, boost::cref(fb)));
  }

  void Schedule::respondToOpenOutputFiles(FileBlock const& fb) {
    for_all(all_workers_, boost::bind(&Worker::respondToOpenOutputFiles, _1, boost::cref(fb)));
  }

  void Schedule::respondToCloseOutputFiles(FileBlock const& fb) {
    for_all(all_workers_, boost::bind(&Worker::respondToCloseOutputFiles, _1, boost::cref(fb)));
  }

  void Schedule::beginJob() {
    for_all(all_workers_, boost::bind(&Worker::beginJob, _1));
  }

  vector<ModuleDescription const*>
  Schedule::getAllModuleDescriptions() const {
    Workers::const_iterator i(workersBegin());
    Workers::const_iterator e(workersEnd());

    vector<ModuleDescription const*> result;
    result.reserve(all_workers_.size());

    for (; i!=e; ++i) {
      ModuleDescription const* p = (*i)->descPtr();
      result.push_back(p);
    }
    return result;
  }

  void
  Schedule::enableEndPaths(bool active) {
    endpathsAreActive_ = active;
  }

  bool
  Schedule::endPathsEnabled() const {
    return endpathsAreActive_;
  }

  void
  fillModuleInPathSummary(Path const&,
                          ModuleInPathSummary&) {
  }


  void
  fillModuleInPathSummary(Path const& path,
                          size_t which,
                          ModuleInPathSummary& sum) {
    sum.timesVisited = path.timesVisited(which);
    sum.timesPassed  = path.timesPassed(which);
    sum.timesFailed  = path.timesFailed(which);
    sum.timesExcept  = path.timesExcept(which);
    sum.moduleLabel  =
      path.getWorker(which)->description().moduleLabel_;
  }

  void
  fillPathSummary(Path const& path, PathSummary& sum) {
    sum.name        = path.name();
    sum.bitPosition = path.bitPosition();
    sum.timesRun    = path.timesRun();
    sum.timesPassed = path.timesPassed();
    sum.timesFailed = path.timesFailed();
    sum.timesExcept = path.timesExcept();

    Path::size_type sz = path.size();
    vector<ModuleInPathSummary> temp(sz);
    for (size_t i = 0; i != sz; ++i) {
      fillModuleInPathSummary(path, i, temp[i]);
    }
    sum.moduleInPathSummaries.swap(temp);
  }

  void
  fillWorkerSummaryAux(Worker const& w, WorkerSummary& sum) {
    sum.timesVisited = w.timesVisited();
    sum.timesRun     = w.timesRun();
    sum.timesPassed  = w.timesPassed();
    sum.timesFailed  = w.timesFailed();
    sum.timesExcept  = w.timesExcept();
    sum.moduleLabel  = w.description().moduleLabel_;
  }

  void
  fillWorkerSummary(Worker const* pw, WorkerSummary& sum) {
    fillWorkerSummaryAux(*pw, sum);
  }

  void
  Schedule::getTriggerReport(TriggerReport& rep) const {
    rep.eventSummary.totalEvents = totalEvents();
    rep.eventSummary.totalEventsPassed = totalEventsPassed();
    rep.eventSummary.totalEventsFailed = totalEventsFailed();

    fill_summary(trig_paths_,  rep.trigPathSummaries, &fillPathSummary);
    fill_summary(end_paths_,   rep.endPathSummaries,  &fillPathSummary);
    fill_summary(all_workers_, rep.workerSummaries,   &fillWorkerSummary);
  }

  void
  Schedule::clearCounters() {
    total_events_ = total_passed_ = 0;
    for_all(trig_paths_, boost::bind(&Path::clearCounters, _1));
    for_all(end_paths_, boost::bind(&Path::clearCounters, _1));
    for_all(all_workers_, boost::bind(&Worker::clearCounters, _1));
  }

  void
  Schedule::getAllWorkers(std::vector<Worker*> &out)
  {
    copy_all(all_workers_, std::back_inserter(out));
  }

  void
  Schedule::resetAll() {
    for_all(all_workers_, boost::bind(&Worker::reset, _1));
    results_->reset();
    endpath_results_->reset();
  }

  void
  Schedule::addToAllWorkers(Worker* w) {
    if (!search_all(all_workers_, w)) all_workers_.push_back(w);
  }

  void
  Schedule::setupOnDemandSystem(EventPrincipal& ep) {
    // NOTE: who owns the productdescrption?  Just copied by value
    ep.setUnscheduledHandler(unscheduled_);
    typedef vector<boost::shared_ptr<ConstBranchDescription const> > branches;
    for(branches::iterator itBranch = demandBranches_.begin(), itBranchEnd = demandBranches_.end();
        itBranch != itBranchEnd;
        ++itBranch) {
      ep.addOnDemandGroup(**itBranch);
    }
  }

   Schedule::WorkerPtr
   Schedule::makeInserter(ParameterSet const& trig_pset) const {

      WorkerParams work_args(pset_,trig_pset,*prod_reg_,*act_table_,processName_);
      ModuleDescription md;
      md.parameterSetID_ = trig_pset.id();
      md.moduleName_ = "TriggerResultInserter";
      md.moduleLabel_ = "TriggerResults";
      md.processConfiguration_ = ProcessConfiguration(processName_, pset_.id(), getReleaseVersion(), getPassID());

      actReg_->preModuleConstructionSignal_(md);
      auto_ptr<EDProducer> producer(new TriggerResultInserter(trig_pset,results_));
      actReg_->postModuleConstructionSignal_(md);

      Schedule::WorkerPtr ptr(new WorkerT<EDProducer>(producer, md, work_args));
      ptr->setActivityRegistry(actReg_);
      return ptr;
   }

   void Schedule::catalogOnDemandBranches(std::set<std::string> const & unscheduledLabels) {
      ProductRegistry::ProductList const& prodsList = prod_reg_->productList();
      for(ProductRegistry::ProductList::const_iterator itProdInfo = prodsList.begin(),
             itProdInfoEnd = prodsList.end();
          itProdInfo != itProdInfoEnd;
          ++itProdInfo) {
         if(processName_ == itProdInfo->second.processName() && itProdInfo->second.branchType() == InEvent &&
            unscheduledLabels.end() != unscheduledLabels.find(itProdInfo->second.moduleLabel())) {
            boost::shared_ptr<ConstBranchDescription const> bd(new ConstBranchDescription(itProdInfo->second));
            demandBranches_.push_back(bd);
         }
      }
   }

   void Schedule::pathConsistencyCheck(size_t expected_num_workers) const {
      // Major sanity check: make sure nobody has added a worker after
      // we've already relied on all_workers_ being full. Failure here
      // indicates a logic error in Schedule().
      assert(expected_num_workers == all_workers_.size() &&
             "INTERNAL ASSERTION ERROR: all_workers_ changed after being used.");

      size_t numFailures = 0;
      numFailures = std::accumulate(trig_paths_.begin(),
                                    trig_paths_.end(),
                                    numFailures,
                                    std::bind(&art::Schedule::accumulateConsistencyFailures,
                                              this,
                                              _1,
                                              _2,
                                              false));
      numFailures = std::accumulate(end_paths_.begin(),
                                    end_paths_.end(),
                                    numFailures,
                                    std::bind(&art::Schedule::accumulateConsistencyFailures,
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

   size_t Schedule::accumulateConsistencyFailures(size_t current_num_failures,
                                                  art::Path const &path,
                                                  bool isEndPath) const {
      return current_num_failures +
         checkOnePath(path, isEndPath);
    }

   size_t Schedule::checkOnePath(Path const &path, bool isEndPath) const {
       std::vector<std::string> results;
      std::ostringstream message;
      if (isEndPath) {
         message << "The following modules are illegal in an end path  (\""
                 << path.name()
                 << "\"): they modify the event "
                 << "and should be in a standard (trigger) path.";
         path.findEventModifiers(results);
      } else {
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

}  // art

// ======================================================================
