#include "art/Framework/EventProcessor/detail/writeSummary.h"

#include "art/Framework/Core/PathManager.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>

using mf::LogAbsolute;
using std::right;
using std::setw;
using std::setprecision;
using std::fixed;

namespace {

  void workersInPathTriggerReport(int const firstBit,
                                  int const bitPosition,
                                  art::Path::WorkersInPath const& workersInPath)
  {
    for (auto const& workerInPath : workersInPath) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5)  << firstBit
                                << right << setw(5)  << bitPosition << " "
                                << right << setw(10) << workerInPath.timesVisited() << " "
                                << right << setw(10) << workerInPath.timesPassed() << " "
                                << right << setw(10) << workerInPath.timesFailed() << " "
                                << right << setw(10) << workerInPath.timesExcept() << " "
                                << workerInPath.getWorker()->description().moduleLabel() << "";
    }
  }

  void workersInEndPathTriggerReport(int const firstBit,
                                  int const bitPosition,
                                  art::Path::WorkersInPath const& workersInPath)
  {
    for (auto const& workerInPath : workersInPath) {
      auto worker = workerInPath.getWorker();
      assert(worker->timesFailed()==0);
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5)  << firstBit
                                << right << setw(5)  << bitPosition << " "
                                << right << setw(10) << worker->timesRun() << " " // proxy for visited
                                << right << setw(10) << worker->timesPassed() << " "
                                << right << setw(10) << worker->timesExcept() << " "
                                << worker->description().moduleLabel() << "";
    }
  }

  void workersInPathTimeReport(unsigned long const totalEvents,
                               art::Path::WorkersInPath const& workersInPath)
  {
    for (auto const& workerInPath : workersInPath) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << workerInPath.timeCpuReal().first / std::max(1ul, totalEvents) << " "
                                << right << setw(10) << workerInPath.timeCpuReal().second / std::max(1ul, totalEvents) << " "
                                << right << setw(10) << workerInPath.timeCpuReal().first / std::max(1ul, workerInPath.timesVisited()) << " "
                                << right << setw(10) << workerInPath.timeCpuReal().second / std::max(1ul, workerInPath.timesVisited()) << " "
                                << workerInPath.getWorker()->description().moduleLabel() << "";
    }
  }
}

void
art::detail::writeSummary(PathManager& pm, bool const wantSummary)
{
  // Still only assuming one schedule. Will need to loop when we get around to it.
  auto const& epi = pm.endPathInfo();
  auto const& tpi = pm.triggerPathsInfo(ScheduleID::first());
  triggerReport(epi, tpi, wantSummary);
  timeReport(epi, tpi, wantSummary);
}

void
art::detail::triggerReport(PathsInfo const& epi, PathsInfo const& tpi, bool const wantSummary)
{
  // The trigger report (pass/fail etc.):
  // Printed even if summary not requested, per issue #1864.
  LogAbsolute("ArtSummary") << "";
  LogAbsolute("ArtSummary") << "TrigReport " << "---------- Event  Summary ------------";
  LogAbsolute("ArtSummary") << "TrigReport"
                            << " Events total = " << tpi.totalEvents()
                            << " passed = " << tpi.passedEvents()
                            << " failed = " << tpi.failedEvents()
                            << "";
  if (wantSummary) {
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " << "---------- Path   Summary ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (auto const& path : tpi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5) << 1
                                << right << setw(5) << path->bitPosition() << " "
                                << right << setw(10) << path->timesRun() << " "
                                << right << setw(10) << path->timesPassed() << " "
                                << right << setw(10) << path->timesFailed() << " "
                                << right << setw(10) << path->timesExcept() << " "
                                << path->name() << "";
    }
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " << "-------End-Path   Summary ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Success" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (auto const& path : epi.pathPtrs()) {
      assert(path->timesFailed()==0);
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5) << 0
                                << right << setw(5) << path->bitPosition() << " "
                                << right << setw(10) << path->timesRun() << " "
                                << right << setw(10) << path->timesPassed() << " "
                                << right << setw(10) << path->timesExcept() << " "
                                << path->name() << "";
    }
    for (auto const& path : tpi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "";
      LogAbsolute("ArtSummary") << "TrigReport " << "---------- Modules in Path: " << path->name() << " ------------";
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(10) << "Trig Bit#" << " "
                                << right << setw(10) << "Visited" << " "
                                << right << setw(10) << "Passed" << " "
                                << right << setw(10) << "Failed" << " "
                                << right << setw(10) << "Error" << " "
                                << "Name" << "";
      workersInPathTriggerReport(1, path->bitPosition(), path->workersInPath());
    }
  }

  // Printed even if summary not requested, per issue #1864.
  for (auto const& path : epi.pathPtrs()) {
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " << "------ Modules in End-Path: " << path->name() << " ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Success" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    workersInEndPathTriggerReport(0, path->bitPosition(), path->workersInPath());
  }

  if (wantSummary) {
    // This table can arguably be removed since all summary
    // information is better described aboved.
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " << "---------- Module Summary ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Visited" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";

    for (auto const& val: tpi.workers()) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(10) << val.second->timesVisited() << " "
                                << right << setw(10) << val.second->timesRun() << " "
                                << right << setw(10) << val.second->timesPassed() << " "
                                << right << setw(10) << val.second->timesFailed() << " "
                                << right << setw(10) << val.second->timesExcept() << " "
                                << val.first << "";
    }

    for (auto const& val: epi.workers()) {
      // Instead of timesVisited(), which is confusing for the user
      // for end-path modules, we just report timesRun() as a proxy
      // for visited.
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(10) << val.second->timesVisited() << " "
                                << right << setw(10) << val.second->timesRun() << " "
                                << right << setw(10) << val.second->timesPassed() << " "
                                << right << setw(10) << val.second->timesFailed() << " "
                                << right << setw(10) << val.second->timesExcept() << " "
                                << val.first << "";
    }
  }
}

void
art::detail::timeReport(PathsInfo const& epi, PathsInfo const& tpi, bool const wantSummary)
{
  LogAbsolute("ArtSummary") << "";
  // The timing report (CPU and Real Time):
  LogAbsolute("ArtSummary") << "TimeReport " << "---------- Time  Summary ---[sec]----";
  LogAbsolute("ArtSummary") << "TimeReport"
                            << setprecision(6) << fixed
                            << " CPU = " << tpi.timeCpuReal().first + epi.timeCpuReal().first
                            << " Real = " << tpi.timeCpuReal().second + epi.timeCpuReal().second
                            << "";
  LogAbsolute("ArtSummary") << "";
  if (wantSummary) {
    LogAbsolute("ArtSummary") << "TimeReport " << "---------- Event  Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport"
                              << setprecision(6) << fixed
                              << " CPU/event = " << (tpi.timeCpuReal().first + epi.timeCpuReal().first) / std::max(1ul, tpi.totalEvents())
                              << " Real/event = " << (tpi.timeCpuReal().second + epi.timeCpuReal().second) / std::max(1ul, tpi.totalEvents())
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TimeReport " << "---------- Path   Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per path-run "
                              << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    for (auto const& path : tpi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << path->timeCpuReal().first / std::max(1ul, tpi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1ul, tpi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().first / std::max(1ul, path->timesRun()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1ul, path->timesRun()) << " "
                                << path->name() << "";
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per path-run "
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TimeReport " << "-------End-Path   Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per endpath-run "
                              << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    for (auto const& path : epi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << path->timeCpuReal().first / std::max(1ul, epi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1ul, epi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().first / std::max(1ul, path->timesRun()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1ul, path->timesRun()) << " "
                                << path->name() << "";
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per endpath-run "
                              << "";
    for (auto const& path : tpi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "";
      LogAbsolute("ArtSummary") << "TimeReport " << "---------- Modules in Path: " << path->name() << " ---[sec]----";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(22) << "per event "
                                << right << setw(22) << "per module-visit "
                                << "";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << "Name" << "";
      workersInPathTimeReport(tpi.totalEvents(), path->workersInPath());
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-visit "
                              << "";
    for (auto const& path : epi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "";
      LogAbsolute("ArtSummary") << "TimeReport " << "------ Modules in End-Path: " << path->name() << " ---[sec]----";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(22) << "per event "
                                << right << setw(22) << "per module-visit "
                                << "";
      LogAbsolute("ArtSummary") << "TimeReport "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << right << setw(10) << "CPU" << " "
                                << right << setw(10) << "Real" << " "
                                << "Name" << "";
      workersInPathTimeReport(tpi.totalEvents(), path->workersInPath());
    }
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-visit "
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TimeReport " << "---------- Module Summary ---[sec]----";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-run "
                              << right << setw(22) << "per module-visit "
                              << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";

    auto workerTimes = [&tpi](WorkerMap::value_type const& val) {
      LogAbsolute("ArtSummary") << "TimeReport "
      << setprecision(6) << fixed
      << right << setw(10) << val.second->timeCpuReal().first / std::max(1ul, tpi.totalEvents()) << " "
      << right << setw(10) << val.second->timeCpuReal().second / std::max(1ul, tpi.totalEvents()) << " "
      << right << setw(10) << val.second->timeCpuReal().first / std::max(1ul, val.second->timesRun()) << " "
      << right << setw(10) << val.second->timeCpuReal().second / std::max(1ul, val.second->timesRun()) << " "
      << right << setw(10) << val.second->timeCpuReal().first / std::max(1ul, val.second->timesVisited()) << " "
      << right << setw(10) << val.second->timeCpuReal().second / std::max(1ul, val.second->timesVisited()) << " "
      << val.first << "";
    };
    cet::for_all(tpi.workers(), workerTimes);
    cet::for_all(epi.workers(), workerTimes);

    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << right << setw(10) << "CPU" << " "
                              << right << setw(10) << "Real" << " "
                              << "Name" << "";
    LogAbsolute("ArtSummary") << "TimeReport "
                              << right << setw(22) << "per event "
                              << right << setw(22) << "per module-run "
                              << right << setw(22) << "per module-visit "
                              << "";
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "T---Report end!" << "";
    LogAbsolute("ArtSummary") << "";
  }
}
