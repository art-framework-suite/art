#include "art/Framework/Core/Schedule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/skip_non_replicated.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

using namespace hep::concurrency;
using namespace std;

using fhicl::ParameterSet;

namespace art {

  Schedule::Schedule(ScheduleID const scheduleID,
                     PathManager& pm,
                     ActionTable const& actions,
                     ActivityRegistry const& actReg,
                     UpdateOutputCallbacks& outputCallbacks,
                     std::unique_ptr<Worker> triggerResultsInserter)
    : epExec_{scheduleID, pm, actions, actReg, outputCallbacks}
    , tpsExec_{scheduleID, pm, actions, move(triggerResultsInserter)}
  {
    TDEBUG_FUNC_SI(5, scheduleID) << hex << this << dec;
  }

  void
  Schedule::beginJob()
  {
    tpsExec_.beginJob();
    epExec_.beginJob();
  }

  void
  Schedule::endJob()
  {
    tpsExec_.endJob();
    epExec_.endJob();
  }

  void
  Schedule::respondToOpenInputFile(FileBlock const& fb)
  {
    tpsExec_.respondToOpenInputFile(fb);
    epExec_.respondToOpenInputFile(fb);
  }

  void
  Schedule::respondToCloseInputFile(FileBlock const& fb)
  {
    tpsExec_.respondToCloseInputFile(fb);
    epExec_.respondToCloseInputFile(fb);
  }

  void
  Schedule::respondToOpenOutputFiles(FileBlock const& fb)
  {
    tpsExec_.respondToOpenOutputFiles(fb);
    epExec_.respondToOpenOutputFiles(fb);
  }

  void
  Schedule::respondToCloseOutputFiles(FileBlock const& fb)
  {
    tpsExec_.respondToCloseOutputFiles(fb);
    epExec_.respondToCloseOutputFiles(fb);
  }

  void
  Schedule::process(Transition const trans, Principal& principal)
  {
    tpsExec_.process(trans, principal);
    epExec_.process(trans, principal);
  }

  // Note: We get here as part of the readAndProcessEvent task.  Our
  // parent task is the nullptr, and the parent task of the
  // endPathTask is the eventLoopTask.
  void
  Schedule::process_event_modifiers(tbb::task* endPathTask,
                                    tbb::task* eventLoopTask,
                                    EventPrincipal& principal)
  {
    tpsExec_.process_event(endPathTask, eventLoopTask, principal);
  }

  void
  Schedule::process_event_observers(EventPrincipal& principal)
  {
    epExec_.process_event(principal);
  }

  // Note: We come here as part of the pathsDone task.  Our parent is
  // the nullptr.
  void
  Schedule::process_event_pathsDone(tbb::task* endPathTask,
                                    tbb::task* eventLoopTask,
                                    EventPrincipal& principal)
  {
    tpsExec_.process_event_pathsDone(endPathTask, eventLoopTask, principal);
  }
} // namespace art
