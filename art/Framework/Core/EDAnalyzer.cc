#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"

using namespace std;

namespace art {

  string
  EDAnalyzer::workerType() const
  {
    return "WorkerT<EDAnalyzer>";
  }

  void
  EDAnalyzer::setupQueues()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    createQueues();
  }

  void
  EDAnalyzer::respondToOpenInputFileWithServices(FileBlock const& fb,
                                                 Services const&)
  {
    respondToOpenInputFile(fb);
  }

  void
  EDAnalyzer::respondToCloseInputFileWithServices(FileBlock const& fb,
                                                  Services const&)
  {
    respondToCloseInputFile(fb);
  }

  void
  EDAnalyzer::respondToOpenOutputFilesWithServices(FileBlock const& fb,
                                                   Services const&)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  EDAnalyzer::respondToCloseOutputFilesWithServices(FileBlock const& fb,
                                                    Services const&)
  {
    respondToCloseOutputFiles(fb);
  }

  void
  EDAnalyzer::beginJobWithServices(Services const&)
  {
    beginJob();
  }

  void
  EDAnalyzer::endJobWithServices(Services const&)
  {
    endJob();
  }

  void
  EDAnalyzer::beginRunWithServices(Run const& r, Services const&)
  {
    beginRun(r);
  }

  void
  EDAnalyzer::endRunWithServices(Run const& r, Services const&)
  {
    endRun(r);
  }

  void
  EDAnalyzer::beginSubRunWithServices(SubRun const& sr, Services const&)
  {
    beginSubRun(sr);
  }

  void
  EDAnalyzer::endSubRunWithServices(SubRun const& sr, Services const&)
  {
    endSubRun(sr);
  }

  void
  EDAnalyzer::analyzeWithServices(Event const& e, Services const& services)
  {
    ScheduleIDSentry sentry{*this, services.scheduleID()};
    analyze(e);
  }

  // Default implementations
  void
  EDAnalyzer::beginJob()
  {}

  void
  EDAnalyzer::endJob()
  {}

  void
  EDAnalyzer::respondToOpenInputFile(FileBlock const&)
  {}

  void
  EDAnalyzer::respondToCloseInputFile(FileBlock const&)
  {}

  void
  EDAnalyzer::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  EDAnalyzer::respondToCloseOutputFiles(FileBlock const&)
  {}

  void
  EDAnalyzer::beginRun(Run const&)
  {}

  void
  EDAnalyzer::endRun(Run const&)
  {}

  void
  EDAnalyzer::beginSubRun(SubRun const&)
  {}

  void
  EDAnalyzer::endSubRun(SubRun const&)
  {}

} // namespace art
