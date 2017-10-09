#include "art/Framework/EventProcessor/detail/writeSummary.h"

#include "art/Framework/Core/PathManager.h"
#include "art/Framework/EventProcessor/detail/memoryReport.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/cpu_timer.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>

using mf::LogPrint;
using std::fixed;
using std::right;
using std::setprecision;
using std::setw;

namespace {

  void
  workersInPathTriggerReport(int const firstBit,
                             int const bitPosition,
                             art::Path::WorkersInPath const& workersInPath)
  {
    for (auto const& workerInPath : workersInPath) {
      LogPrint("ArtSummary")
        << "TrigReport " << right << setw(5) << firstBit << right << setw(5)
        << bitPosition << " " << right << setw(10)
        << workerInPath.timesVisited() << " " << right << setw(10)
        << workerInPath.timesPassed() << " " << right << setw(10)
        << workerInPath.timesFailed() << " " << right << setw(10)
        << workerInPath.timesExcept() << " "
        << workerInPath.getWorker()->description().moduleLabel();
    }
  }

  void
  workersInEndPathTriggerReport(int const firstBit,
                                int const bitPosition,
                                art::Path::WorkersInPath const& workersInPath)
  {
    for (auto const& workerInPath : workersInPath) {
      auto worker = workerInPath.getWorker();
      LogPrint("ArtSummary")
        << "TrigReport " << right << setw(5) << firstBit << right << setw(5)
        << bitPosition << " " << right << setw(10) << worker->timesRun()
        << " " // proxy for visited
        << right << setw(10) << worker->timesPassed() << " " << right
        << setw(10) << worker->timesExcept() << " "
        << worker->description().moduleLabel();
    }
  }
}

void
art::detail::writeSummary(PathManager& pm,
                          bool const wantSummary,
                          cet::cpu_timer const& jobTimer)
{
  // Still only assuming one schedule. Will need to loop when we get around to
  // it.
  auto const& epi = pm.endPathInfo();
  auto const& tpi = pm.triggerPathsInfo(ScheduleID::first());
  LogPrint("ArtSummary") << "";
  triggerReport(epi, tpi, wantSummary);
  LogPrint("ArtSummary") << "";
  timeReport(jobTimer);
  LogPrint("ArtSummary") << "";
  memoryReport();
}

void
art::detail::triggerReport(PathsInfo const& epi,
                           PathsInfo const& tpi,
                           bool const wantSummary)
{
  // The trigger report (pass/fail etc.):
  // Printed even if summary not requested, per issue #1864.
  LogPrint("ArtSummary") << "TrigReport "
                         << "---------- Event  Summary ------------";
  LogPrint("ArtSummary") << "TrigReport"
                         << " Events total = " << tpi.totalEvents()
                         << " passed = " << tpi.passedEvents()
                         << " failed = " << tpi.failedEvents();
  if (wantSummary) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- Path   Summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << right << setw(10) << "Trig Bit#"
                           << " " << right << setw(10) << "Run"
                           << " " << right << setw(10) << "Passed"
                           << " " << right << setw(10) << "Failed"
                           << " " << right << setw(10) << "Error"
                           << " "
                           << "Name";
    for (auto const& path : tpi.pathPtrs()) {
      LogPrint("ArtSummary")
        << "TrigReport " << right << setw(5) << 1 << right << setw(5)
        << path->bitPosition() << " " << right << setw(10) << path->timesRun()
        << " " << right << setw(10) << path->timesPassed() << " " << right
        << setw(10) << path->timesFailed() << " " << right << setw(10)
        << path->timesExcept() << " " << path->name();
    }
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "-------End-Path   Summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << right << setw(10) << "Trig Bit#"
                           << " " << right << setw(10) << "Run"
                           << " " << right << setw(10) << "Success"
                           << " " << right << setw(10) << "Error"
                           << " "
                           << "Name";
    for (auto const& path : epi.pathPtrs()) {
      LogPrint("ArtSummary")
        << "TrigReport " << right << setw(5) << 0 << right << setw(5)
        << path->bitPosition() << " " << right << setw(10) << path->timesRun()
        << " " << right << setw(10) << path->timesPassed() << " " << right
        << setw(10) << path->timesExcept() << " " << path->name();
    }
    for (auto const& path : tpi.pathPtrs()) {
      LogPrint("ArtSummary") << "";
      LogPrint("ArtSummary")
        << "TrigReport "
        << "---------- Modules in Path: " << path->name() << " ------------";
      LogPrint("ArtSummary")
        << "TrigReport " << right << setw(10) << "Trig Bit#"
        << " " << right << setw(10) << "Visited"
        << " " << right << setw(10) << "Passed"
        << " " << right << setw(10) << "Failed"
        << " " << right << setw(10) << "Error"
        << " "
        << "Name";
      workersInPathTriggerReport(1, path->bitPosition(), path->workersInPath());
    }
  }

  // Printed even if summary not requested, per issue #1864.
  for (auto const& path : epi.pathPtrs()) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "------ Modules in End-Path: " << path->name()
                           << " ------------";
    LogPrint("ArtSummary") << "TrigReport " << right << setw(10) << "Trig Bit#"
                           << " " << right << setw(10) << "Run"
                           << " " << right << setw(10) << "Success"
                           << " " << right << setw(10) << "Error"
                           << " "
                           << "Name";
    workersInEndPathTriggerReport(
      0, path->bitPosition(), path->workersInPath());
  }

  if (wantSummary) {
    // This table can arguably be removed since all summary
    // information is better described aboved.
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport "
                           << "---------- Module Summary ------------";
    LogPrint("ArtSummary") << "TrigReport " << right << setw(10) << "Visited"
                           << " " << right << setw(10) << "Run"
                           << " " << right << setw(10) << "Passed"
                           << " " << right << setw(10) << "Failed"
                           << " " << right << setw(10) << "Error"
                           << " "
                           << "Name";

    for (auto const& val : tpi.workers()) {
      LogPrint("ArtSummary")
        << "TrigReport " << right << setw(10) << val.second->timesVisited()
        << " " << right << setw(10) << val.second->timesRun() << " " << right
        << setw(10) << val.second->timesPassed() << " " << right << setw(10)
        << val.second->timesFailed() << " " << right << setw(10)
        << val.second->timesExcept() << " " << val.first;
    }

    for (auto const& val : epi.workers()) {
      // Instead of timesVisited(), which is confusing for the user
      // for end-path modules, we just report timesRun() as a proxy
      // for visited.
      LogPrint("ArtSummary")
        << "TrigReport " << right << setw(10) << val.second->timesVisited()
        << " " << right << setw(10) << val.second->timesRun() << " " << right
        << setw(10) << val.second->timesPassed() << " " << right << setw(10)
        << val.second->timesFailed() << " " << right << setw(10)
        << val.second->timesExcept() << " " << val.first;
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
