#include "art/Framework/Core/ReplicatedAnalyzer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  ReplicatedAnalyzer::workerType() const
  {
    return "WorkerT<ReplicatedAnalyzer>";
  }

  void
  ReplicatedAnalyzer::setupQueues()
  {
    // For art 3.0, replicated modules will not have queues.
  }

  void
  ReplicatedAnalyzer::respondToOpenInputFileWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToOpenInputFile(fb, services);
  }

  void
  ReplicatedAnalyzer::respondToCloseInputFileWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseInputFile(fb, services);
  }

  void
  ReplicatedAnalyzer::respondToOpenOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToOpenOutputFiles(fb, services);
  }

  void
  ReplicatedAnalyzer::respondToCloseOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseOutputFiles(fb, services);
  }

  void
  ReplicatedAnalyzer::beginJobWithServices(Services const& services)
  {
    beginJob(services);
  }

  void
  ReplicatedAnalyzer::endJobWithServices(Services const& services)
  {
    endJob(services);
  }

  void
  ReplicatedAnalyzer::beginRunWithServices(Run const& r,
                                           Services const& services)
  {
    beginRun(r, services);
  }

  void
  ReplicatedAnalyzer::endRunWithServices(Run const& r, Services const& services)
  {
    endRun(r, services);
  }

  void
  ReplicatedAnalyzer::beginSubRunWithServices(SubRun const& sr,
                                              Services const& services)
  {
    beginSubRun(sr, services);
  }

  void
  ReplicatedAnalyzer::endSubRunWithServices(SubRun const& sr,
                                            Services const& services)
  {
    endSubRun(sr, services);
  }

  void
  ReplicatedAnalyzer::analyzeWithServices(Event const& e,
                                          Services const& services)
  {
    analyze(e, services);
  }

  void
  ReplicatedAnalyzer::beginJob(Services const&)
  {}

  void
  ReplicatedAnalyzer::endJob(Services const&)
  {}

  void
  ReplicatedAnalyzer::respondToOpenInputFile(FileBlock const&, Services const&)
  {}

  void
  ReplicatedAnalyzer::respondToCloseInputFile(FileBlock const&, Services const&)
  {}

  void
  ReplicatedAnalyzer::respondToOpenOutputFiles(FileBlock const&,
                                               Services const&)
  {}

  void
  ReplicatedAnalyzer::respondToCloseOutputFiles(FileBlock const&,
                                                Services const&)
  {}

  void
  ReplicatedAnalyzer::beginRun(Run const&, Services const&)
  {}

  void
  ReplicatedAnalyzer::endRun(Run const&, Services const&)
  {}

  void
  ReplicatedAnalyzer::beginSubRun(SubRun const&, Services const&)
  {}

  void
  ReplicatedAnalyzer::endSubRun(SubRun const&, Services const&)
  {}

} // namespace art
