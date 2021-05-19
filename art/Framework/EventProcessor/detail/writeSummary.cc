#include "art/Framework/EventProcessor/detail/writeSummary.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/EventProcessor/detail/memoryReport.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "cetlib/cpu_timer.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <vector>

using mf::LogPrint;
using std::fixed;
using std::setprecision;
using std::setw;

namespace {

  struct SummaryCounts {
    std::size_t total{};
    std::size_t passed{};
    std::size_t failed{};
  };

  struct TriggerCounts {
    std::string path_name{};
    std::size_t run{};
    std::size_t passed{};
    std::size_t failed{};
    std::size_t except{};
  };

  struct EndPathCounts {
    std::size_t run{};
    std::size_t success{};
    std::size_t except{};
  };

  struct WorkerInPathCounts {
    std::string moduleLabel{};
    std::size_t visited{};
    std::size_t passed{};
    std::size_t failed{};
    std::size_t except{};
  };

  struct ModuleCounts {
    std::size_t visited{};
    std::size_t run{};
    std::size_t passed{};
    std::size_t failed{};
    std::size_t except{};
  };

  using WorkersInPath = std::vector<art::WorkerInPath>;
  using WorkersInPathCounts = std::vector<WorkerInPathCounts>;

  void
  workersInPathTriggerReport(art::PathID const path_id,
                             WorkersInPathCounts const& workersInPathCounts)
  {
    for (auto const& wip_counts : workersInPathCounts) {
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << to_string(path_id) << " "
        << std::right << setw(10) << wip_counts.visited << " " << std::right
        << setw(10) << wip_counts.passed << " " << std::right << setw(10)
        << wip_counts.failed << " " << std::right << setw(10)
        << wip_counts.except << " " << wip_counts.moduleLabel;
    }
  }

  void
  workersInEndPathTriggerReport(WorkersInPathCounts const& workersInPathCounts)
  {
    for (auto const& wip_counts : workersInPathCounts) {
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << wip_counts.visited << " "
        << std::right << setw(10) << wip_counts.passed << " " << std::right
        << setw(10) << wip_counts.except << " " << wip_counts.moduleLabel;
    }
  }

} // unnamed namespace

void
art::detail::writeSummary(PathManager& pm,
                          bool const wantSummary,
                          cet::cpu_timer const& jobTimer)
{
  auto const& epis = pm.endPathInfo();
  auto const& tpis = pm.triggerPathsInfo();
  LogPrint("ArtSummary") << "";
  triggerReport(epis, tpis, wantSummary);
  LogPrint("ArtSummary") << "";
  timeReport(jobTimer);
  LogPrint("ArtSummary") << "";
  memoryReport();
}

void
art::detail::triggerReport(PerScheduleContainer<PathsInfo> const& epis,
                           PerScheduleContainer<PathsInfo> const& tpis,
                           bool const wantSummary)
{
  // Checking the first element is sufficient since the path
  // structures are identical across schedules/end-path executors.
  auto observers_enabled = !epis[ScheduleID::first()].paths().empty();

  SummaryCounts total_counts{};
  for (auto const& tpi : tpis) {
    total_counts.total += tpi.totalEvents();
    total_counts.passed += tpi.passedEvents();
    total_counts.failed += tpi.failedEvents();
  }
  // The trigger report (pass/fail etc.):
  // Printed even if summary not requested, per issue #1864.
  LogPrint("ArtSummary") << "TrigReport "
                         << "---------- Event summary -------------";
  LogPrint("ArtSummary") << "TrigReport"
                         << " Events total = " << total_counts.total
                         << " passed = " << total_counts.passed
                         << " failed = " << total_counts.failed;

  if (wantSummary) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- Trigger-path summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << std::right << setw(10)
                           << "Path ID"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Passed"
                           << " " << std::right << setw(10) << "Failed"
                           << " " << std::right << setw(10) << "Error"
                           << " "
                           << "Name";
    std::map<PathID, TriggerCounts> counts_per_path{};
    for (auto const& tpi : tpis) {
      for (auto const& path : tpi.paths()) {
        auto& counts = counts_per_path[path.pathID()];
        counts.path_name = path.name(); // No increment!
        counts.run += path.timesRun();
        counts.passed += path.timesPassed();
        counts.failed += path.timesFailed();
        counts.except += path.timesExcept();
      }
    }
    for (auto const& [pathID, counts] : counts_per_path) {
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << to_string(pathID) << " "
        << std::right << setw(10) << counts.run << " " << std::right << setw(10)
        << counts.passed << " " << std::right << setw(10) << counts.failed
        << " " << std::right << setw(10) << counts.except << " "
        << counts.path_name;
    }

    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- End-path summary ---------";
    LogPrint("ArtSummary") << "TrigReport"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Success"
                           << " " << std::right << setw(10) << "Error";

    if (observers_enabled) {
      EndPathCounts epCounts{};
      for (auto const& epi : epis) {
        for (auto const& path : epi.paths()) {
          epCounts.run += path.timesRun();
          epCounts.success += path.timesPassed();
          epCounts.except += path.timesExcept();
        }
      }
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << epCounts.run << " "
        << std::right << setw(10) << epCounts.success << " " << std::right
        << setw(10) << epCounts.except;
    }

    std::map<PathSpec, WorkersInPathCounts> counts_per_worker_in_path;
    for (auto const& tpi : tpis) {
      for (auto const& path : tpi.paths()) {
        auto& counts_per_worker = counts_per_worker_in_path[path.pathSpec()];
        if (counts_per_worker.empty() && !path.workersInPath().empty()) {
          counts_per_worker.resize(path.workersInPath().size());
        }
        std::size_t i{};
        for (auto const& workerInPath : path.workersInPath()) {
          auto const* worker = workerInPath.getWorker();
          auto& counts = counts_per_worker[i];
          if (counts.moduleLabel.empty()) {
            counts.moduleLabel = worker->description().moduleLabel();
          }
          counts.visited += workerInPath.timesVisited();
          counts.passed += workerInPath.timesPassed();
          counts.failed += workerInPath.timesFailed();
          counts.except += workerInPath.timesExcept();
          ++i;
        }
      }
    }
    for (auto const& [path_spec, worker_in_path_counts] :
         counts_per_worker_in_path) {
      LogPrint("ArtSummary") << "";
      LogPrint("ArtSummary")
        << "TrigReport "
        << "---------- Modules in path: " << path_spec.name << " ------------";
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << "Path ID"
        << " " << std::right << setw(10) << "Visited"
        << " " << std::right << setw(10) << "Passed"
        << " " << std::right << setw(10) << "Failed"
        << " " << std::right << setw(10) << "Error"
        << " "
        << "Name";
      workersInPathTriggerReport(path_spec.path_id, worker_in_path_counts);
    }
  }

  // Printed even if summary not requested, per issue #1864.
  WorkersInPathCounts endPathWIPCounts;
  for (auto const& epi : epis) {
    for (auto const& path : epi.paths()) {
      if (endPathWIPCounts.empty() && !path.workersInPath().empty()) {
        endPathWIPCounts.resize(path.workersInPath().size());
      }
      std::size_t i{};
      for (auto const& workerInPath : path.workersInPath()) {
        auto const* worker = workerInPath.getWorker();
        auto& counts = endPathWIPCounts[i];
        if (counts.moduleLabel.empty()) {
          counts.moduleLabel = worker->description().moduleLabel();
        }
        counts.visited += worker->timesRun(); // proxy for 'visited'
        counts.passed += worker->timesPassed();
        counts.except += worker->timesExcept();
        ++i;
      }
    }
  }

  if (observers_enabled) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- Modules in End-path ----------";
    LogPrint("ArtSummary") << "TrigReport"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Success"
                           << " " << std::right << setw(10) << "Error"
                           << " "
                           << "Name";
    workersInEndPathTriggerReport(endPathWIPCounts);
  }

  if (wantSummary) {
    // This table can arguably be removed since all summary
    // information is better described above.
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- Module summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << std::right << setw(10)
                           << "Visited"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Passed"
                           << " " << std::right << setw(10) << "Failed"
                           << " " << std::right << setw(10) << "Error"
                           << " "
                           << "Name";
    std::map<std::string, ModuleCounts> counts_per_module;
    auto update_counts = [&counts_per_module](auto const& pathInfos) {
      for (auto const& pi : pathInfos) {
        for (auto const& [module_label, module_counts] : pi.workers()) {
          auto& counts = counts_per_module[module_label];
          counts.visited += module_counts->timesVisited();
          counts.run += module_counts->timesRun();
          counts.passed += module_counts->timesPassed();
          counts.failed += module_counts->timesFailed();
          counts.except += module_counts->timesExcept();
        }
      }
    };
    update_counts(tpis);
    update_counts(epis);
    for (auto const& [module_label, module_counts] : counts_per_module) {
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << module_counts.visited
        << " " << std::right << setw(10) << module_counts.run << " "
        << std::right << setw(10) << module_counts.passed << " " << std::right
        << setw(10) << module_counts.failed << " " << std::right << setw(10)
        << module_counts.except << " " << module_label;
    }
  }
}

void
art::detail::timeReport(cet::cpu_timer const& timer)
{
  LogPrint("ArtSummary") << "TimeReport "
                         << "---------- Time summary [sec] -------";
  LogPrint("ArtSummary") << "TimeReport " << setprecision(6) << fixed
                         << "CPU = " << timer.cpuTime()
                         << " Real = " << timer.realTime();
}
