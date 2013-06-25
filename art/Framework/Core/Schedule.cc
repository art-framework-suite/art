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
#include "cpp0x/numeric"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iomanip>

// Commonly-used (and unambiguous) symbols from other namespaces.
using fhicl::ParameterSet;
using std::placeholders::_1;

art::Schedule::
Schedule(ScheduleID sID,
         PathManager & pm,
         ParameterSet const & proc_pset,
         art::TriggerNamesService const & tns,
         MasterProductRegistry & pregistry,
         ActionTable & actions,
         std::shared_ptr<ActivityRegistry> areg):
  sID_(sID),
  process_pset_(proc_pset),
  act_table_(&actions),
  processName_(tns.getProcessName()),
  actReg_(areg),
  triggerPathsInfo_(pm.triggerPathsInfo(sID_)),
  results_inserter_(),
  demand_branches_()
{
  assert(actReg_);
  if (!triggerPathsInfo_.pathPtrs().empty()) {
    makeTriggerResultsInserter(tns.getTriggerPSet(), pregistry);
    // Deal with on-demand registration.
    if (process_pset_.get<bool>("services.scheduler.allowUnscheduled", false)) {
      BranchesByModuleLabel branchLookup;
      fillBranchLookup(pregistry.productList(), branchLookup);
      catalogOnDemandBranches(branchLookup);
    }
  }
  pregistry.setFrozen();
  if (sID == ScheduleID::first()) {
    ProductMetaData::create_instance(pregistry);
  }
} // art::Schedule::Schedule

void
art::Schedule::
endJob()
{
  bool failure = false;
  Exception error(errors::EndJobFailure);
  doForAllWorkers_
    ([&failure, &error](Worker * w)
     {
       try {
         w->endJob();
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
     });
  if (failure) { throw error; }
}

void
art::Schedule::
respondToOpenInputFile(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToOpenInputFile, _1,
                             std::cref(fb)));
}

void
art::Schedule::
respondToCloseInputFile(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToCloseInputFile, _1,
                             std::cref(fb)));
}

void
art::Schedule::
respondToOpenOutputFiles(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToOpenOutputFiles, _1,
                                  std::cref(fb)));
}

void
art::Schedule::
respondToCloseOutputFiles(FileBlock const & fb)
{
  doForAllWorkers_(std::bind(&Worker::respondToCloseOutputFiles, _1,
                                  std::cref(fb)));
}

void
art::Schedule::
beginJob()
{
  doForAllWorkers_(std::bind(&Worker::beginJob, _1));
}

void
art::Schedule::
resetAll()
{
  doForAllWorkers_(std::bind(&Worker::reset, _1));
  triggerPathsInfo_.pathResults().reset();
}

// FIXME: This can work with generic Principals just as soon as the
// metadata can handle (or obviate) a BranchID <-> ProductID
// conversion for all principal types.
void
art::Schedule::
setupOnDemandSystem(EventPrincipal & p)
{
  BranchType b(p.branchType());
  for (auto & val : demand_branches_) {
    if (val.second->branchType() == b) {
      p.addOnDemandGroup(*val.second, val.first);
    }
  }
}

void
art::Schedule::
makeTriggerResultsInserter(ParameterSet const & trig_pset,
                                          MasterProductRegistry & pregistry)
{
  WorkerParams work_args(process_pset_, trig_pset, pregistry, *act_table_,
                         processName_);
  ModuleDescription md(trig_pset.id(),
                       "TriggerResultInserter",
                       "TriggerResults",
                       ProcessConfiguration(processName_,
                                            process_pset_.id(),
                                            getReleaseVersion(),
                                            getPassID()));
  actReg_->sPreModuleConstruction.invoke(md);
  std::unique_ptr<EDProducer>
    producer(new TriggerResultInserter(trig_pset, triggerPathsInfo_.pathResults()));
  actReg_->sPostModuleConstruction.invoke(md);
  results_inserter_.reset(new WorkerT<EDProducer>(std::move(producer), md,
                                                  work_args));
  results_inserter_->setActivityRegistry(actReg_);
}

void
art::Schedule::
fillBranchLookup(ProductList const & pList,
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

void
art::Schedule::
catalogOnDemandBranches(BranchesByModuleLabel const & branchLookup)
{
  for (auto const & val : triggerPathsInfo_.workers()) {
    BranchesByModuleLabel::const_iterator
      lb(branchLookup.lower_bound(val.first)),
      e(branchLookup.end()),
      ub(branchLookup.upper_bound(val.first));
    if (lb == ub) { return; } // This worker produces nothing.
    for (BranchesByModuleLabel::const_iterator i = lb;
         i != ub;
         ++i) {
      demand_branches_.emplace(typename OnDemandBranches::key_type(val.second.get()), i->second);
    }
  }
}

void
art::Schedule::
doForAllWorkers_(std::function<void (Worker *)> func)
{
  for (auto const & val : triggerPathsInfo_.workers()) {
    func(val.second.get());
  }
  func(results_inserter_.get()); // Do this last -- not part of main list.
}

void
art::Schedule::
doForAllPaths_(std::function<void (Path *)> func)
{
  for (auto const & path : triggerPathsInfo_.pathPtrs()) {
    func(path.get());
  }
}
