#include "art/Framework/EventProcessor/Scheduler.h"
// vim: set sw=2 expandtab :

#include "art/Utilities/Globals.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/getenv.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cstdlib>
#include <string>

using fhicl::ParameterSet;

namespace {
  unsigned
  adjust_num_threads(unsigned const specified_num_threads)
  {
    char const* p = std::getenv("OMP_NUM_THREADS");
    if (p == nullptr) {
      return specified_num_threads;
    }

    auto const max_threads = std::stoul(p);
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

  constexpr auto max_parallelism = tbb::global_control::max_allowed_parallelism;
  constexpr auto thread_stack_size = tbb::global_control::thread_stack_size;
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

  std::unique_ptr<GlobalTaskGroup>
  Scheduler::global_task_group()
  {
    using tbb::global_control;
    auto value_of = [](auto const field) {
      return global_control::active_value(field);
    };
    auto group = std::make_unique<GlobalTaskGroup>(nThreads_, stackSize_);
    mf::LogInfo("MTdiagnostics")
      << "TBB has been configured to use:\n"
      << "  - a maximum of " << value_of(max_parallelism) << " threads\n"
      << "  - a stack size of " << value_of(thread_stack_size) << " bytes";
    return group;
  }
}
