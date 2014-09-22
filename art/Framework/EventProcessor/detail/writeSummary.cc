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

void
art::detail::writeSummary(PathManager & pm, bool wantSummary)
{
  // Still only assuming one schedule. Will need to loop when we get around to it.
  auto const & epi = pm.endPathInfo();
  auto const & tpi = pm.triggerPathsInfo(ScheduleID::first());

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
    for (auto const & path : tpi.pathPtrs()) {
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
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (auto const & path : epi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5) << 0
                                << right << setw(5) << path->bitPosition() << " "
                                << right << setw(10) << path->timesRun() << " "
                                << right << setw(10) << path->timesPassed() << " "
                                << right << setw(10) << path->timesFailed() << " "
                                << right << setw(10) << path->timesExcept() << " "
                                << path->name() << "";
    }
    for (auto const & path : tpi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "";
      LogAbsolute("ArtSummary") << "TrigReport " << "---------- Modules in Path: " << path->name() << " ------------";
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(10) << "Trig Bit#" << " "
                                << right << setw(10) << "Visited" << " "
                                << right << setw(10) << "Passed" << " "
                                << right << setw(10) << "Failed" << " "
                                << right << setw(10) << "Error" << " "
                                << "Name" << "";
      for (unsigned int i = 0; i < path->size(); ++i) {
        LogAbsolute("ArtSummary") << "TrigReport "
                                  << right << setw(5) << 1
                                  << right << setw(5) << path->bitPosition() << " "
                                  << right << setw(10) << path->timesVisited(i) << " "
                                  << right << setw(10) << path->timesPassed(i) << " "
                                  << right << setw(10) << path->timesFailed(i) << " "
                                  << right << setw(10) << path->timesExcept(i) << " "
                                  << path->getWorker(i)->description().moduleLabel() << "";
      }
    }
  }
  // Printed even if summary not requested, per issue #1864.
  for (auto const & path : epi.pathPtrs()) {
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " << "------ Modules in End-Path: " << path->name() << " ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Trig Bit#" << " "
                              << right << setw(10) << "Visited" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    for (unsigned int i = 0; i < path->size(); ++i) {
      LogAbsolute("ArtSummary") << "TrigReport "
                                << right << setw(5) << 0
                                << right << setw(5) << path->bitPosition() << " "
                                << right << setw(10) << path->timesVisited(i) << " "
                                << right << setw(10) << path->timesPassed(i) << " "
                                << right << setw(10) << path->timesFailed(i) << " "
                                << right << setw(10) << path->timesExcept(i) << " "
                                << path->getWorker(i)->description().moduleLabel() << "";
    }
  }
  if (wantSummary) {
    LogAbsolute("ArtSummary") << "";
    LogAbsolute("ArtSummary") << "TrigReport " << "---------- Module Summary ------------";
    LogAbsolute("ArtSummary") << "TrigReport "
                              << right << setw(10) << "Visited" << " "
                              << right << setw(10) << "Run" << " "
                              << right << setw(10) << "Passed" << " "
                              << right << setw(10) << "Failed" << " "
                              << right << setw(10) << "Error" << " "
                              << "Name" << "";
    auto workerstats = [](WorkerMap::value_type const & val) {
      LogAbsolute("ArtSummary") << "TrigReport "
      << right << setw(10) << val.second->timesVisited() << " "
      << right << setw(10) << val.second->timesRun() << " "
      << right << setw(10) << val.second->timesPassed() << " "
      << right << setw(10) << val.second->timesFailed() << " "
      << right << setw(10) << val.second->timesExcept() << " "
      << val.first << "";
    };
    cet::for_all(tpi.workers(), workerstats);
    cet::for_all(epi.workers(), workerstats);
  }
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
    for (auto const & path : tpi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << path->timeCpuReal().first / std::max(1ul, tpi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1ul, tpi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().first / std::max(1, path->timesRun()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1, path->timesRun()) << " "
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
    for (auto const & path : epi.pathPtrs()) {
      LogAbsolute("ArtSummary") << "TimeReport "
                                << setprecision(6) << fixed
                                << right << setw(10) << path->timeCpuReal().first / std::max(1ul, epi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1ul, epi.totalEvents()) << " "
                                << right << setw(10) << path->timeCpuReal().first / std::max(1, path->timesRun()) << " "
                                << right << setw(10) << path->timeCpuReal().second / std::max(1, path->timesRun()) << " "
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
    for (auto const & path : tpi.pathPtrs()) {
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
      for (unsigned int i = 0; i < path->size(); ++i) {
        LogAbsolute("ArtSummary") << "TimeReport "
                                  << setprecision(6) << fixed
                                  << right << setw(10) << path->timeCpuReal(i).first / std::max(1ul, tpi.totalEvents()) << " "
                                  << right << setw(10) << path->timeCpuReal(i).second / std::max(1ul, tpi.totalEvents()) << " "
                                  << right << setw(10) << path->timeCpuReal(i).first / std::max(1, path->timesVisited(i)) << " "
                                  << right << setw(10) << path->timeCpuReal(i).second / std::max(1, path->timesVisited(i)) << " "
                                  << path->getWorker(i)->description().moduleLabel() << "";
      }
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
    for (auto const & path : epi.pathPtrs()) {
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
      for (unsigned int i = 0; i < path->size(); ++i) {
        LogAbsolute("ArtSummary") << "TimeReport "
                                  << setprecision(6) << fixed
                                  << right << setw(10) << path->timeCpuReal(i).first / std::max(1ul, epi.totalEvents()) << " "
                                  << right << setw(10) << path->timeCpuReal(i).second / std::max(1ul, epi.totalEvents()) << " "
                                  << right << setw(10) << path->timeCpuReal(i).first / std::max(1, path->timesVisited(i)) << " "
                                  << right << setw(10) << path->timeCpuReal(i).second / std::max(1, path->timesVisited(i)) << " "
                                  << path->getWorker(i)->description().moduleLabel() << "";
      }
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
    auto workertimes = [&tpi](WorkerMap::value_type const & val) {
      LogAbsolute("ArtSummary") << "TimeReport "
      << setprecision(6) << fixed
      << right << setw(10) << val.second->timeCpuReal().first / std::max(1ul, tpi.totalEvents()) << " "
      << right << setw(10) << val.second->timeCpuReal().second / std::max(1ul, tpi.totalEvents()) << " "
      << right << setw(10) << val.second->timeCpuReal().first / std::max(1, val.second->timesRun()) << " "
      << right << setw(10) << val.second->timeCpuReal().second / std::max(1, val.second->timesRun()) << " "
      << right << setw(10) << val.second->timeCpuReal().first / std::max(1, val.second->timesVisited()) << " "
      << right << setw(10) << val.second->timeCpuReal().second / std::max(1, val.second->timesVisited()) << " "
      << val.first << "";
    };
    cet::for_all(tpi.workers(), workertimes);
    cet::for_all(epi.workers(), workertimes);
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

