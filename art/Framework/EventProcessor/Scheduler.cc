#include "art/Framework/EventProcessor/Scheduler.h"
// vim: set sw=2 expandtab :

#include "art/Utilities/Globals.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/getenv.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"
#include "tbb/task_arena.h"

#include <cstdlib>
#include <string>

using fhicl::ParameterSet;

namespace {
  int
  adjust_num_threads(int const specified_num_threads)
  {
    char const* p = std::getenv("OMP_NUM_THREADS");
    if (p == nullptr) {
      return specified_num_threads;
    }

    auto const max_threads = std::stoi(p);
    if (specified_num_threads == 0) {
      return max_threads;
    }
    if (specified_num_threads <= max_threads) {
      return specified_num_threads;
    }

    cet::HorizontalRule const rule{80};
    mf::LogAbsolute("MTconfig")
      << rule('=') << '\n'
      << "The specified number of threads (" << specified_num_threads
      << ") exceeds the allowed number (" << max_threads << ").\n"
      << "The allowed number of threads (" << max_threads
      << ") will be used for this job.\n"
      << rule('=');
    return max_threads;
  }
}

namespace art {

  Scheduler::Scheduler(Parameters const& ps)
    : actionTable_{ps().actionTable()}
    , nThreads_{adjust_num_threads(ps().num_threads())}
    , nSchedules_{ps().num_schedules()}
    , stackSize_{ps().stack_size()}
    , handleEmptyRuns_{ps().handleEmptyRuns()}
    , handleEmptySubRuns_{ps().handleEmptySubRuns()}
    , errorOnMissingConsumes_{ps().errorOnMissingConsumes()}
    , wantSummary_{ps().wantSummary()}
    , dataDependencyGraph_{ps().dataDependencyGraph()}
  {
    auto& globals = *Globals::instance();
    globals.setNThreads(nThreads_);
    globals.setNSchedules(nSchedules_);
  }

  void
  Scheduler::initialize_task_manager()
  {
    tbbManager_.initialize(nThreads_, stackSize_);
    mf::LogInfo("MTdiagnostics")
      << "TBB has been configured to use:\n"
      << "  - a maximum of " << tbb::this_task_arena::max_concurrency()
      << " threads\n"
      << "  - a stack size of " << stackSize_ << " bytes";
  }
}
