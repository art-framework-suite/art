#include "art/Framework/Core/Schedule.h"
// vim: set sw=2:

//
// Controls the execution of paths and endpaths.
//

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "cetlib/exempt_ptr.h"

#include <algorithm>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <numeric>

using fhicl::ParameterSet;

art::Schedule::
Schedule(ScheduleID sID, PathManager& pm, ParameterSet const& proc_pset,
         TriggerNamesService const& tns, MasterProductRegistry& mpr,
         ActionTable& actions, ActivityRegistry& areg)
  : sID_(sID)
  , process_pset_(proc_pset)
  , act_table_(&actions)
  , processName_(tns.getProcessName())
  , triggerPathsInfo_(pm.triggerPathsInfo(sID_))
  , pathsEnabled_(triggerPathsInfo_.pathPtrs().size(), true)
  , results_inserter_()
  , demand_branches_(catalogOnDemandBranches_(pm.onDemandWorkers(),
                                              mpr.productList()))
{
  if (!triggerPathsInfo_.pathPtrs().empty()) {
    makeTriggerResultsInserter_(tns.getTriggerPSet(), mpr, areg);
  }
  mpr.setFrozen();
  if (sID == ScheduleID::first()) {
    ProductMetaData::create_instance(mpr);
  }
}

void
art::Schedule::
endJob()
{
  bool failure = false;
  Exception error(errors::EndJobFailure);
  doForAllWorkers_
  ([&failure, &error](Worker * w) {
    try {
      w->endJob();
    }
    catch (cet::exception& e) {
      error << "cet::exception caught in Schedule::endJob\n"
            << e.explain_self();
      failure = true;
    }
    catch (std::exception& e) {
      error << "Standard library exception caught in Schedule::endJob\n"
            << e.what();
      failure = true;
    }
    catch (...) {
      error << "Unknown exception caught in Schedule::endJob\n";
      failure = true;
    }
  });
  if (failure) {
    throw error;
  }
}

void
art::Schedule::
respondToOpenInputFile(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) {
    w->respondToOpenInputFile(fb);
  });
}

void
art::Schedule::
respondToCloseInputFile(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) {
    w->respondToCloseInputFile(fb);
  });
}

void
art::Schedule::
respondToOpenOutputFiles(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) {
    w->respondToOpenOutputFiles(fb);
  });
}

void
art::Schedule::
respondToCloseOutputFiles(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) {
    w->respondToCloseOutputFiles(fb);
  });
}

bool
art::Schedule::
setTriggerPathEnabled(std::string const& name, bool enable)
{
  auto& pp = triggerPathsInfo_.pathPtrs();
  PathPtrs::iterator found;
  auto pathFinder =
  [&name](std::unique_ptr<Path> const & p_ptr) {
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
  }
  else {
    throw Exception(errors::ScheduleExecutionFailure)
        << "Attempt to "
        << (enable ? "enable" : "disable")
        << " unconfigured path "
        << name
        << ".\n";
  }
}

void
art::Schedule::
beginJob()
{
  doForAllWorkers_([](auto w) {
      w->beginJob();
    });
}

art::Schedule::OnDemandBranches
art::Schedule::
catalogOnDemandBranches_(PathManager::Workers onDemandWorkers,
                         ProductList const & plist)
{
  OnDemandBranches result;
  std::multimap<std::string, BranchDescription const *>
    branchLookup;
  for (auto I = plist.cbegin(),
            E = plist.cend(); I != E; ++I) {
    branchLookup.emplace(I->second.moduleLabel(), &I->second);
  }
  for (auto w : onDemandWorkers) {
    auto const& label = w->description().moduleLabel();
    for (auto I = branchLookup.lower_bound(label),
              E = branchLookup.upper_bound(label); I != E; ++I) {
      result.emplace(w, I->second);
    }
  }
  return result;
}

void
art::Schedule::
makeTriggerResultsInserter_(fhicl::ParameterSet const & trig_pset,
                            MasterProductRegistry & mpr,
                            ActivityRegistry & areg)
{
  WorkerParams work_args(process_pset_, trig_pset, mpr, *act_table_,
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
    producer(new TriggerResultInserter(trig_pset,
                                       triggerPathsInfo_.pathResults()));
  results_inserter_.reset(new WorkerT<EDProducer>(std::move(producer), md,
                                                  work_args));
  areg.sPostModuleConstruction.invoke(md);
  results_inserter_->setActivityRegistry(cet::exempt_ptr<ActivityRegistry>
                                         (&areg));
}

