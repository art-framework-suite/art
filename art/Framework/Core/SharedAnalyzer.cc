#include "art/Framework/Core/SharedAnalyzer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  SharedAnalyzer::workerType() const
  {
    return "WorkerT<SharedAnalyzer>";
  }

  void
  SharedAnalyzer::setupQueues()
  {
    createQueues();
  }

  void
  SharedAnalyzer::respondToOpenInputFileWithServices(FileBlock const& fb,
                                                     Services const& services)
  {
    respondToOpenInputFile(fb, services);
  }

  void
  SharedAnalyzer::respondToCloseInputFileWithServices(FileBlock const& fb,
                                                      Services const& services)
  {
    respondToCloseInputFile(fb, services);
  }

  void
  SharedAnalyzer::respondToOpenOutputFilesWithServices(FileBlock const& fb,
                                                       Services const& services)
  {
    respondToOpenOutputFiles(fb, services);
  }

  void
  SharedAnalyzer::respondToCloseOutputFilesWithServices(
    FileBlock const& fb,
    Services const& services)
  {
    respondToCloseOutputFiles(fb, services);
  }

  void
  SharedAnalyzer::beginJobWithServices(Services const& services)
  {
    beginJob(services);
  }

  void
  SharedAnalyzer::endJobWithServices(Services const& services)
  {
    endJob(services);
  }

  void
  SharedAnalyzer::beginRunWithServices(Run const& r, Services const& services)
  {
    beginRun(r, services);
  }

  void
  SharedAnalyzer::endRunWithServices(Run const& r, Services const& services)
  {
    endRun(r, services);
  }

  void
  SharedAnalyzer::beginSubRunWithServices(SubRun const& sr,
                                          Services const& services)
  {
    beginSubRun(sr, services);
  }

  void
  SharedAnalyzer::endSubRunWithServices(SubRun const& sr,
                                        Services const& services)
  {
    endSubRun(sr, services);
  }

  void
  SharedAnalyzer::analyzeWithServices(Event const& e, Services const& services)
  {
    analyze(e, services);
  }

  void
  SharedAnalyzer::beginJob(Services const&)
  {}

  void
  SharedAnalyzer::endJob(Services const&)
  {}

  void
  SharedAnalyzer::respondToOpenInputFile(FileBlock const&, Services const&)
  {}

  void
  SharedAnalyzer::respondToCloseInputFile(FileBlock const&, Services const&)
  {}

  void
  SharedAnalyzer::respondToOpenOutputFiles(FileBlock const&, Services const&)
  {}

  void
  SharedAnalyzer::respondToCloseOutputFiles(FileBlock const&, Services const&)
  {}

  void
  SharedAnalyzer::beginRun(Run const&, Services const&)
  {}

  void
  SharedAnalyzer::endRun(Run const&, Services const&)
  {}

  void
  SharedAnalyzer::beginSubRun(SubRun const&, Services const&)
  {}

  void
  SharedAnalyzer::endSubRun(SubRun const&, Services const&)
  {}

} // namespace art
