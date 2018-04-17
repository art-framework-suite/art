#include "art/Framework/EventProcessor/Scheduler.h"
// vim: set sw=2 expandtab :

#include "art/Utilities/Globals.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"
#include "tbb/task_arena.h"

using fhicl::ParameterSet;

namespace art {

  Scheduler::Scheduler(Parameters const& ps)
    : actionTable_{ps().actionTable()}
    , nThreads_{ps().num_threads()}
    , nSchedules_{ps().num_schedules()}
    , handleEmptyRuns_{ps().handleEmptyRuns()}
    , handleEmptySubRuns_{ps().handleEmptySubRuns()}
    , errorOnMissingConsumes_{ps().errorOnMissingConsumes()}
    , wantSummary_{ps().wantSummary()}
    , dataDependencyGraph_{ps().dataDependencyGraph()}
  {
    auto& globals = *Globals::instance();
    tbbManager_.initialize(nThreads_);
    mf::LogInfo("MTdiagnostics")
      << "TBB has been configured to use a maximum of "
      << tbb::this_task_arena::max_concurrency() << " threads.";
    globals.setNThreads(nThreads_);
    globals.setNSchedules(nSchedules_);
  }
}
