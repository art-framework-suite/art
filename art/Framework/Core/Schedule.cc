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
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "cetlib/exempt_ptr.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

using fhicl::ParameterSet;

art::Schedule::Schedule(ScheduleID const sID,
                        PathManager& pm,
                        ParameterSet const& proc_pset,
                        TriggerNamesService const& tns,
                        MasterProductRegistry& mpr,
                        ActionTable& actions,
                        ActivityRegistry& areg)
  : sID_{sID}
  , process_pset_{proc_pset}
  , act_table_{&actions}
  , processName_{tns.getProcessName()}
  , triggerPathsInfo_{pm.triggerPathsInfo(sID_)}
  , pathsEnabled_(triggerPathsInfo_.pathPtrs().size(), true)
{
  if (!triggerPathsInfo_.pathPtrs().empty()) {
    makeTriggerResultsInserter_(tns.getTriggerPSet(), mpr, areg);
  }
}

void
art::Schedule::endJob()
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
art::Schedule::respondToOpenInputFile(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) { w->respondToOpenInputFile(fb); });
}

void
art::Schedule::respondToCloseInputFile(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) { w->respondToCloseInputFile(fb); });
}

void
art::Schedule::respondToOpenOutputFiles(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) { w->respondToOpenOutputFiles(fb); });
}

void
art::Schedule::respondToCloseOutputFiles(FileBlock const& fb)
{
  doForAllWorkers_([&fb](auto w) { w->respondToCloseOutputFiles(fb); });
}

void
art::Schedule::beginJob()
{
  doForAllWorkers_([](auto w) { w->beginJob(); });
}

void
art::Schedule::makeTriggerResultsInserter_(fhicl::ParameterSet const& trig_pset,
                                           MasterProductRegistry& mpr,
                                           ActivityRegistry& areg)
{
  WorkerParams const work_args{process_pset_, trig_pset, mpr, *act_table_, processName_};
  ModuleDescription md(trig_pset.id(),
                       "TriggerResultInserter",
                       "TriggerResults",
                       ProcessConfiguration(processName_,
                                            process_pset_.id(),
                                            getReleaseVersion()));
  areg.sPreModuleConstruction.invoke(md);
  auto producer = std::make_unique<TriggerResultInserter>(trig_pset, triggerPathsInfo_.pathResults());
  results_inserter_ = std::make_unique<WorkerT<EDProducer>>(std::move(producer), md, work_args);
  areg.sPostModuleConstruction.invoke(md);
  results_inserter_->setActivityRegistry(cet::exempt_ptr<ActivityRegistry>(&areg));
}
