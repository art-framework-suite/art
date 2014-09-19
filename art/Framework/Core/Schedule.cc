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
#include <iomanip>

// Commonly-used (and unambiguous) symbols from other namespaces.
using fhicl::ParameterSet;

art::Schedule::
Schedule(ScheduleID sID,
         PathManager & pm,
         ParameterSet const & proc_pset,
         art::TriggerNamesService const & tns,
         MasterProductRegistry & pregistry,
         ActionTable & actions,
         ActivityRegistry & areg):
  sID_(sID),
  process_pset_(proc_pset),
  act_table_(&actions),
  processName_(tns.getProcessName()),
  triggerPathsInfo_(pm.triggerPathsInfo(sID_)),
  pathsEnabled_(triggerPathsInfo_.pathPtrs().size(), true),
  results_inserter_(),
  demand_branches_(catalogOnDemandBranches_(pm.onDemandWorkers(),
                                            pregistry.productList()))
{
  if (!triggerPathsInfo_.pathPtrs().empty()) {
    makeTriggerResultsInserter_(tns.getTriggerPSet(), pregistry, areg);
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
  doForAllWorkers_([&fb](auto w){ w->respondToOpenInputFile(fb); });
}

void
art::Schedule::
respondToCloseInputFile(FileBlock const & fb)
{
  doForAllWorkers_([&fb](auto w){ w->respondToCloseInputFile(fb); });
}

void
art::Schedule::
respondToOpenOutputFiles(FileBlock const & fb)
{
  doForAllWorkers_([&fb](auto w){ w->respondToOpenOutputFiles(fb); });
}

void
art::Schedule::
respondToCloseOutputFiles(FileBlock const & fb)
{
  doForAllWorkers_([&fb](auto w){ w->respondToCloseOutputFiles(fb); });
}

bool
art::Schedule::
setTriggerPathEnabled(std::string const & name, bool enable)
{
  auto & pp = triggerPathsInfo_.pathPtrs();
  PathPtrs::iterator found;
  auto pathFinder =
    [&name](std::unique_ptr<Path> const & p_ptr)
    {
      return p_ptr->name() == name;
    };
  if ((found =
      std::find_if(pp.begin(),
                   pp.end(),
                   pathFinder)) != pp.end()) {
    size_t index = std::distance(pp.begin(), found);
    auto result = pathsEnabled_[index];
    pathsEnabled_[index] = enable;
    return result;
  } else {
    throw Exception(errors::ScheduleExecutionFailure)
      << "Attempt to "
      << (enable?"enable":"disable")
      << " unconfigured path "
      << name
      << ".\n";
  }
}

void
art::Schedule::
beginJob()
{
  doForAllWorkers_([](auto w){ w->beginJob(); });
}

void
art::Schedule::
resetAll_()
{
  doForAllWorkers_([](auto w){ w->reset(); });
  triggerPathsInfo_.pathResults().reset();
}

// FIXME: This can work with generic Principals just as soon as the
// metadata can handle (or obviate) a BranchID <-> ProductID
// conversion for all principal types.
void
art::Schedule::
setupOnDemandSystem_(EventPrincipal & p)
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
makeTriggerResultsInserter_(ParameterSet const & trig_pset,
                           MasterProductRegistry & pregistry,
                            ActivityRegistry & areg)
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
  areg.sPreModuleConstruction.invoke(md);
  std::unique_ptr<EDProducer>
    producer(new TriggerResultInserter(trig_pset, triggerPathsInfo_.pathResults()));
  areg.sPostModuleConstruction.invoke(md);
  results_inserter_.reset(new WorkerT<EDProducer>(std::move(producer), md,
                                                  work_args));
  results_inserter_->setActivityRegistry(cet::exempt_ptr<ActivityRegistry>(&areg));
}

void
art::Schedule::
fillBranchLookup_(ProductList const & pList,
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

art::Schedule::OnDemandBranches
art::Schedule::
catalogOnDemandBranches_(PathManager::Workers && workers,
                         ProductList const & plist)
{
  OnDemandBranches result;
  BranchesByModuleLabel branchLookup;
  fillBranchLookup_(plist, branchLookup);
  for (auto w : workers) {
    auto const & label = w->description().moduleLabel();
    BranchesByModuleLabel::const_iterator
      lb(branchLookup.lower_bound(label)),
      ub(branchLookup.upper_bound(label));
    for (BranchesByModuleLabel::const_iterator i = lb;
         i != ub;
         ++i) {
      result.emplace(typename OnDemandBranches::key_type(w), i->second);
    }
  }
  return std::move(result);
}
