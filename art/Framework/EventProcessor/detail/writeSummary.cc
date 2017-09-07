#include "art/Framework/EventProcessor/detail/writeSummary.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/EventProcessor/detail/memoryReport.h"
#include "cetlib/cpu_timer.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <vector>

using namespace std;

using mf::LogPrint;

namespace art {
namespace detail {

namespace {

void
workersInPathTriggerReport(int const firstBit, int const bitPosition, vector<WorkerInPath> const& workersInPath)
{
  for (auto const& workerInPath : workersInPath) {
    LogPrint("ArtSummary") << "TrigReport "
                              << std::right << setw(5)  << firstBit
                              << std::right << setw(5)  << bitPosition << " "
                              << std::right << setw(10) << workerInPath.timesVisited() << " "
                              << std::right << setw(10) << workerInPath.timesPassed() << " "
                              << std::right << setw(10) << workerInPath.timesFailed() << " "
                              << std::right << setw(10) << workerInPath.timesExcept() << " "
                              << workerInPath.getWorker()->description().moduleLabel() << "";
  }
}

void
workersInEndPathTriggerReport(int const firstBit, int const bitPosition, vector<WorkerInPath> const& workersInPath)
{
  for (auto const& workerInPath : workersInPath) {
    auto worker = workerInPath.getWorker();
    assert(worker->timesFailed()==0);
    LogPrint("ArtSummary") << "TrigReport "
                              << std::right << setw(5)  << firstBit
                              << std::right << setw(5)  << bitPosition << " "
                              << std::right << setw(10) << worker->timesRun() << " " // proxy for visited
                              << std::right << setw(10) << worker->timesPassed() << " "
                              << std::right << setw(10) << worker->timesExcept() << " "
                              << worker->description().moduleLabel() << "";
  }
}

} // unnamed namespace

void
writeSummary(PathManager& pm, bool const wantSummary, cet::cpu_timer const& jobTimer)
{
  // Still only assuming one schedule. Will need to loop when we get around to it.
  auto const& epi = pm.endPathInfo();
  auto const& tpi = pm.triggerPathsInfo(0);
  LogPrint("ArtSummary") << "";
  triggerReport(epi, tpi, wantSummary);
  LogPrint("ArtSummary") << "";
  timeReport(jobTimer);
  LogPrint("ArtSummary") << "";
  memoryReport();
}

void
triggerReport(PathsInfo const& epi, PathsInfo const& tpi, bool const wantSummary)
{
  // The trigger report (pass/fail etc.):
  // Printed even if summary not requested, per issue #1864.
  LogPrint("ArtSummary") << "TrigReport " << "---------- Event  Summary ------------";
  LogPrint("ArtSummary") << "TrigReport"
                            << " Events total = " << tpi.totalEvents()
                            << " passed = " << tpi.passedEvents()
                            << " failed = " << tpi.failedEvents()
                            << "";
  if (wantSummary) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport " << "---------- Path   Summary ------------";
    LogPrint("ArtSummary") << "TrigReport "
                              << std::right << setw(10) << "Trig Bit#" << " "
                              << std::right << setw(10) << "Run" << " "
                              << std::right << setw(10) << "Passed" << " "
                              << std::right << setw(10) << "Failed" << " "
                              << std::right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (auto const& path : tpi.paths()) {
      LogPrint("ArtSummary") << "TrigReport "
                                << std::right << setw(5) << 1
                                << std::right << setw(5)  << path->bitPosition() << " "
                                << std::right << setw(10) << path->timesRun() << " "
                                << std::right << setw(10) << path->timesPassed() << " "
                                << std::right << setw(10) << path->timesFailed() << " "
                                << std::right << setw(10) << path->timesExcept() << " "
                                << path->name() << "";
    }
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport " << "-------End-Path   Summary ------------";
    LogPrint("ArtSummary") << "TrigReport "
                              << std::right << setw(10) << "Trig Bit#" << " "
                              << std::right << setw(10) << "Run" << " "
                              << std::right << setw(10) << "Success" << " "
                              << std::right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (auto const& path : epi.paths()) {
      assert(path->timesFailed()==0);
      LogPrint("ArtSummary") << "TrigReport "
                                << std::right << setw(5) << 0
                                << std::right << setw(5) << path->bitPosition() << " "
                                << std::right << setw(10) << path->timesRun() << " "
                                << std::right << setw(10) << path->timesPassed() << " "
                                << std::right << setw(10) << path->timesExcept() << " "
                                << path->name() << "";
    }
    for (auto const& path : tpi.paths()) {
      LogPrint("ArtSummary") << "";
      LogPrint("ArtSummary") << "TrigReport " << "---------- Modules in Path: " << path->name() << " ------------";
      LogPrint("ArtSummary") << "TrigReport "
                                << std::right << setw(10) << "Trig Bit#" << " "
                                << std::right << setw(10) << "Visited" << " "
                                << std::right << setw(10) << "Passed" << " "
                                << std::right << setw(10) << "Failed" << " "
                                << std::right << setw(10) << "Error" << " "
                                << "Name" << "";
      workersInPathTriggerReport(1, path->bitPosition(), path->workersInPath());
    }
  }

  // Printed even if summary not requested, per issue #1864.
  for (auto const& path : epi.paths()) {
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport " << "------ Modules in End-Path: " << path->name() << " ------------";
    LogPrint("ArtSummary") << "TrigReport "
                              << std::right << setw(10) << "Trig Bit#" << " "
                              << std::right << setw(10) << "Run" << " "
                              << std::right << setw(10) << "Success" << " "
                              << std::right << setw(10) << "Error" << " "
                              << "Name" << "";
    workersInEndPathTriggerReport(0, path->bitPosition(), path->workersInPath());
  }

  if (wantSummary) {
    // This table can arguably be removed since all summary
    // information is better described above.
    LogPrint("ArtSummary") << "";
    LogPrint("ArtSummary") << "TrigReport " << "---------- Module Summary ------------";
    LogPrint("ArtSummary") << "TrigReport "
                              << std::right << setw(10) << "Visited" << " "
                              << std::right << setw(10) << "Run" << " "
                              << std::right << setw(10) << "Passed" << " "
                              << std::right << setw(10) << "Failed" << " "
                              << std::right << setw(10) << "Error" << " "
                              << "Name" << "";

    for (auto const& val: tpi.workers()) {
      LogPrint("ArtSummary") << "TrigReport "
                                << std::right << setw(10) << val.second->timesVisited() << " "
                                << std::right << setw(10) << val.second->timesRun() << " "
                                << std::right << setw(10) << val.second->timesPassed() << " "
                                << std::right << setw(10) << val.second->timesFailed() << " "
                                << std::right << setw(10) << val.second->timesExcept() << " "
                                << val.first << "";
    }

    for (auto const& val: epi.workers()) {
      // Instead of timesVisited(), which is confusing for the user
      // for end-path modules, we just report timesRun() as a proxy
      // for visited.
      LogPrint("ArtSummary") << "TrigReport "
                                << std::right << setw(10) << val.second->timesVisited() << " "
                                << std::right << setw(10) << val.second->timesRun() << " "
                                << std::right << setw(10) << val.second->timesPassed() << " "
                                << std::right << setw(10) << val.second->timesFailed() << " "
                                << std::right << setw(10) << val.second->timesExcept() << " "
                                << val.first << "";
    }
  }
}

void
timeReport(cet::cpu_timer const& timer)
{
  LogPrint("ArtSummary") << "TimeReport " << "---------- Time  Summary ---[sec]----";
  LogPrint("ArtSummary") << "TimeReport"
                            << setprecision(6) << fixed
                            << " CPU = " << timer.cpuTime() << " Real = " << timer.realTime();
}

} // namespace detail
} // namespace art

