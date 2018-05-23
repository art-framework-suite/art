#include "art/Framework/Core/SharedFilter.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  SharedFilter::workerType() const
  {
    return "WorkerT<SharedFilter>";
  }

  void
  SharedFilter::setupQueues()
  {
    createQueues();
  }

  void
  SharedFilter::respondToOpenInputFileWithServices(FileBlock const& fb,
                                                   Services const& services)
  {
    respondToOpenInputFile(fb, services);
  }

  void
  SharedFilter::respondToCloseInputFileWithServices(FileBlock const& fb,
                                                    Services const& services)
  {
    respondToCloseInputFile(fb, services);
  }

  void
  SharedFilter::respondToOpenOutputFilesWithServices(FileBlock const& fb,
                                                     Services const& services)
  {
    respondToOpenOutputFiles(fb, services);
  }

  void
  SharedFilter::respondToCloseOutputFilesWithServices(FileBlock const& fb,
                                                      Services const& services)
  {
    respondToCloseOutputFiles(fb, services);
  }

  void
  SharedFilter::beginJobWithServices(Services const& services)
  {
    beginJob(services);
  }

  void
  SharedFilter::endJobWithServices(Services const& services)
  {
    endJob(services);
  }

  bool
  SharedFilter::beginRunWithServices(Run& r, Services const& services)
  {
    return beginRun(r, services);
  }

  bool
  SharedFilter::endRunWithServices(Run& r, Services const& services)
  {
    return endRun(r, services);
  }

  bool
  SharedFilter::beginSubRunWithServices(SubRun& sr, Services const& services)
  {
    return beginSubRun(sr, services);
  }

  bool
  SharedFilter::endSubRunWithServices(SubRun& sr, Services const& services)
  {
    return endSubRun(sr, services);
  }

  bool
  SharedFilter::filterWithServices(Event& e, Services const& services)
  {
    return filter(e, services);
  }

  // Default implementations
  void
  SharedFilter::beginJob(Services const&)
  {}

  void
  SharedFilter::endJob(Services const&)
  {}

  void
  SharedFilter::respondToOpenInputFile(FileBlock const&, Services const&)
  {}

  void
  SharedFilter::respondToCloseInputFile(FileBlock const&, Services const&)
  {}

  void
  SharedFilter::respondToOpenOutputFiles(FileBlock const&, Services const&)
  {}

  void
  SharedFilter::respondToCloseOutputFiles(FileBlock const&, Services const&)
  {}

  bool
  SharedFilter::beginRun(Run&, Services const&)
  {
    return true;
  }

  bool
  SharedFilter::endRun(Run&, Services const&)
  {
    return true;
  }

  bool
  SharedFilter::beginSubRun(SubRun&, Services const&)
  {
    return true;
  }

  bool
  SharedFilter::endSubRun(SubRun&, Services const&)
  {
    return true;
  }

} // namespace art
