#include "art/Framework/EventProcessor/detail/writeSummary.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/EventProcessor/detail/memoryReport.h"
#include "cetlib/container_algorithms.h"
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
    int bitPosition{};
    std::size_t run{};
    std::size_t passed{};
    std::size_t failed{};
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
  workersInPathTriggerReport(int const firstBit,
                             int const bitPosition,
                             WorkersInPathCounts const& workersInPathCounts)
  {
    for (auto const& wip_counts : workersInPathCounts) {
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(5) << firstBit << std::right
        << setw(5) << bitPosition << " " << std::right << setw(10)
        << wip_counts.visited << " " << std::right << setw(10)
        << wip_counts.passed << " " << std::right << setw(10)
        << wip_counts.failed << " " << std::right << setw(10)
        << wip_counts.except << " " << wip_counts.moduleLabel;
    }
  }

  void
  workersInEndPathTriggerReport(int const firstBit,
                                int const bitPosition,
                                WorkersInPath const& workersInPath)
  {
    for (auto const& workerInPath : workersInPath) {
      auto worker = workerInPath.getWorker();
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(5) << firstBit << std::right
        << setw(5) << bitPosition << " " << std::right << setw(10)
        << worker->timesRun() << " " // proxy for visited
        << std::right << setw(10) << worker->timesPassed() << " " << std::right
        << setw(10) << worker->timesExcept() << " "
        << worker->description().moduleLabel();
    }
  }

} // unnamed namespace

void
art::detail::writeSummary(PathManager& pm,
                          bool const wantSummary,
                          cet::cpu_timer const& jobTimer)
{
  auto const& epi = pm.endPathInfo();
  auto const& tpis = pm.triggerPathsInfo();
  LogPrint("ArtSummary") << "";
  triggerReport(epi, tpis, wantSummary);
  LogPrint("ArtSummary") << "";
  timeReport(jobTimer);
  LogPrint("ArtSummary") << "";
  memoryReport();
}

void
art::detail::triggerReport(PathsInfo const& epi,
                           std::vector<PathsInfo> const& tpis,
                           bool const wantSummary)
{
  SummaryCounts total_counts{};
  for (auto const& tpi : tpis) {
    total_counts.total += tpi.totalEvents();
    total_counts.passed += tpi.passedEvents();
    total_counts.failed += tpi.failedEvents();
  }

  // The trigger report (pass/fail etc.):
  // Printed even if summary not requested, per issue #1864.
  LogPrint("ArtSummary") << "TrigReport "
                         << "---------- Event  Summary ------------";
  LogPrint("ArtSummary") << "TrigReport"
                         << " Events total = " << total_counts.total
                         << " passed = " << total_counts.passed
                         << " failed = " << total_counts.failed;
  if (wantSummary) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- Path   Summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << std::right << setw(10)
                           << "Trig Bit#"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Passed"
                           << " " << std::right << setw(10) << "Failed"
                           << " " << std::right << setw(10) << "Error"
                           << " "
                           << "Name";
    std::map<std::string, TriggerCounts> counts_per_path{};
    for (auto const& tpi : tpis) {
      for (auto const& path : tpi.paths()) {
        auto& counts = counts_per_path[path->name()];
        counts.bitPosition = path->bitPosition(); // No increment!
        counts.run += path->timesRun();
        counts.passed += path->timesPassed();
        counts.failed += path->timesFailed();
        counts.except += path->timesExcept();
      }
    }
    for (auto const& pr : counts_per_path) {
      auto const& path_name = pr.first;
      auto const& counts = pr.second;
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(5) << 1 << std::right << setw(5)
        << counts.bitPosition << " " << std::right << setw(10) << counts.run
        << " " << std::right << setw(10) << counts.passed << " " << std::right
        << setw(10) << counts.failed << " " << std::right << setw(10)
        << counts.except << " " << path_name;
    }
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "-------End-Path   Summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << std::right << setw(10)
                           << "Trig Bit#"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Success"
                           << " " << std::right << setw(10) << "Error"
                           << " "
                           << "Name";
    for (auto const& path : epi.paths()) {
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(5) << 0 << std::right << setw(5)
        << path->bitPosition() << " " << std::right << setw(10)
        << path->timesRun() << " " << std::right << setw(10)
        << path->timesPassed() << " " << std::right << setw(10)
        << path->timesExcept() << " " << path->name();
    }

    // std::tuple<...> guarantees weak-ordering, so it is a suitable
    // key type for an std::map.
    using path_data_t = std::tuple<std::string, int>; // path-name, bit position
    std::map<path_data_t, WorkersInPathCounts> counts_per_worker_in_path;
    for (auto const& tpi : tpis) {
      for (auto const& path : tpi.paths()) {
        path_data_t const path_data{path->name(), path->bitPosition()};
        auto& counts_per_worker = counts_per_worker_in_path[path_data];
        if (counts_per_worker.empty() && !path->workersInPath().empty()) {
          counts_per_worker.resize(path->workersInPath().size());
        }
        std::size_t i{};
        for (auto const& workerInPath : path->workersInPath()) {
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

    for (auto const& pr : counts_per_worker_in_path) {
      auto const& path_data = pr.first;
      auto const& path_name = std::get<std::string>(path_data);
      auto const& bit_position = std::get<int>(path_data);
      auto const& worker_in_path_counts = pr.second;
      LogPrint("ArtSummary") << "";
      LogPrint("ArtSummary")
        << "TrigReport "
        << "---------- Modules in Path: " << path_name << " ------------";
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << "Trig Bit#"
        << " " << std::right << setw(10) << "Visited"
        << " " << std::right << setw(10) << "Passed"
        << " " << std::right << setw(10) << "Failed"
        << " " << std::right << setw(10) << "Error"
        << " "
        << "Name";
      workersInPathTriggerReport(1, bit_position, worker_in_path_counts);
    }
  }

  // Printed even if summary not requested, per issue #1864.
  for (auto const& path : epi.paths()) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "------ Modules in End-Path: " << path->name()
                           << " ------------";
    LogPrint("ArtSummary") << "TrigReport " << std::right << setw(10)
                           << "Trig Bit#"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Success"
                           << " " << std::right << setw(10) << "Error"
                           << " "
                           << "Name";
    workersInEndPathTriggerReport(
      0, path->bitPosition(), path->workersInPath());
  }

  if (wantSummary) {
    // This table can arguably be removed since all summary
    // information is better described above.
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- Module Summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << std::right << setw(10)
                           << "Visited"
                           << " " << std::right << setw(10) << "Run"
                           << " " << std::right << setw(10) << "Passed"
                           << " " << std::right << setw(10) << "Failed"
                           << " " << std::right << setw(10) << "Error"
                           << " "
                           << "Name";

    std::map<std::string, ModuleCounts> counts_per_module;
    for (auto const& tpi : tpis) {
      for (auto const& pr : tpi.workers()) {
        auto const& module_label = pr.first;
        auto const& module_counts = *pr.second;
        auto& counts = counts_per_module[module_label];
        counts.visited += module_counts.timesVisited();
        counts.run += module_counts.timesRun();
        counts.passed += module_counts.timesPassed();
        counts.failed += module_counts.timesFailed();
        counts.except += module_counts.timesExcept();
      }
    }
    for (auto const& pr : counts_per_module) {
      auto const& module_label = pr.first;
      auto const& module_counts = pr.second;
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << module_counts.visited
        << " " << std::right << setw(10) << module_counts.run << " "
        << std::right << setw(10) << module_counts.passed << " " << std::right
        << setw(10) << module_counts.failed << " " << std::right << setw(10)
        << module_counts.except << " " << module_label;
    }

    for (auto const& val : epi.workers()) {
      // Instead of timesVisited(), which is confusing for the user
      // for end-path modules, we just report timesRun() as a proxy
      // for visited.
      LogPrint("ArtSummary")
        << "TrigReport " << std::right << setw(10) << val.second->timesVisited()
        << " " << std::right << setw(10) << val.second->timesRun() << " "
        << std::right << setw(10) << val.second->timesPassed() << " "
        << std::right << setw(10) << val.second->timesFailed() << " "
        << std::right << setw(10) << val.second->timesExcept() << " "
        << val.first;
    }
  }
}

void
art::detail::timeReport(cet::cpu_timer const& timer)
{
  LogPrint("ArtSummary") << "TimeReport "
                         << "---------- Time  Summary ---[sec]----";
  LogPrint("ArtSummary") << "TimeReport " << setprecision(6) << fixed
                         << "CPU = " << timer.cpuTime()
                         << " Real = " << timer.realTime();
}
