#include "art/Framework/Core/Schedule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "hep_concurrency/WaitingTask.h"

#include <ios>

using namespace hep::concurrency;
using namespace std;

namespace art {

  Schedule::Schedule(ScheduleID const scheduleID,
                     PathManager& pm,
                     ActionTable const& actions,
                     ActivityRegistry const& actReg,
                     UpdateOutputCallbacks& outputCallbacks,
                     GlobalTaskGroup& task_group)
    : context_{scheduleID}
    , actions_{actions}
    , epExec_{scheduleID, pm, actions, outputCallbacks, task_group}
    , tpsExec_{scheduleID, pm, actions, actReg, task_group}
  {
    TDEBUG_FUNC_SI(5, scheduleID) << hex << this << dec;
  }

  void
  Schedule::beginJob(detail::SharedResources const& resources)
  {
    tpsExec_.beginJob(resources);
    epExec_.beginJob(resources);
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

  void
  Schedule::process_event_modifiers(WaitingTaskPtr endPathTask)
  {
    tpsExec_.process_event(endPathTask, *eventPrincipal_);
  }

  void
  Schedule::process_event_observers(WaitingTaskPtr finalizeEventTask)
  {
    epExec_.process_event(finalizeEventTask, *eventPrincipal_);
  }

} // namespace art
