#include "art/Framework/Core/OutputWorker.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art {

  OutputWorker::~OutputWorker() = default;

  // This is called directly by the make_worker function created
  // by the DEFINE_ART_MODULE macro.
  OutputWorker::OutputWorker(std::shared_ptr<OutputModule> mod,
                             ModuleDescription const& md,
                             WorkerParams const& wp)
    : WorkerT<OutputModule>{mod, md, wp}
  {
    ci_->outputModuleInitiated(
      label(),
      fhicl::ParameterSetRegistry::get(description().parameterSetID()));
  }

  std::string const&
  OutputWorker::lastClosedFileName() const
  {
    return module().lastClosedFileName();
  }

  bool
  OutputWorker::closeFile()
  {
    bool const didCloseFile{module().doCloseFile()};
    if (didCloseFile) {
      ci_->outputFileClosed(description().moduleLabel(), lastClosedFileName());
    }
    return didCloseFile;
  }

  void
  OutputWorker::incrementInputFileNumber()
  {
    return module().incrementInputFileNumber();
  }

  bool
  OutputWorker::requestsToCloseFile() const
  {
    return module().requestsToCloseFile();
  }

  bool
  OutputWorker::openFile(FileBlock const& fb)
  {
    bool const didOpenFile{module().doOpenFile(fb)};
    if (didOpenFile) {
      ci_->outputFileOpened(description().moduleLabel());
    }
    return didOpenFile;
  }

  void
  OutputWorker::writeRun(RunPrincipal& rp)
  {
    module().doWriteRun(rp);
  }

  void
  OutputWorker::writeSubRun(SubRunPrincipal& srp)
  {
    module().doWriteSubRun(srp);
  }

  void
  OutputWorker::writeEvent(EventPrincipal& ep, ModuleContext const& mc)
  {
    module().doWriteEvent(ep, mc);
  }

  void
  OutputWorker::setRunAuxiliaryRangeSetID(RangeSet const& rs)
  {
    module().doSetRunAuxiliaryRangeSetID(rs);
  }

  void
  OutputWorker::setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
  {
    module().doSetSubRunAuxiliaryRangeSetID(rs);
  }

  bool
  OutputWorker::fileIsOpen() const
  {
    return module().fileIsOpen();
  }

  void
  OutputWorker::setFileStatus(OutputFileStatus const ofs)
  {
    return module().setFileStatus(ofs);
  }

  bool
  OutputWorker::limitReached() const
  {
    return module().limitReached();
  }

  void
  OutputWorker::configure(OutputModuleDescription const& desc)
  {
    module().configure(desc);
  }

  void
  OutputWorker::selectProducts(ProductTables const& tables)
  {
    module().selectProducts(tables);
  }

  Granularity
  OutputWorker::fileGranularity() const
  {
    return module().fileGranularity();
  }

} // namespace art
