#include "art/Framework/Core/OutputWorker.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art {

  OutputWorker::~OutputWorker() = default;

  // This is called directly by the make_worker function created
  // by the DEFINE_ART_MODULE macro.
  OutputWorker::OutputWorker(std::shared_ptr<OutputModule> module,
                             ModuleDescription const& md,
                             WorkerParams const& wp)
    : Worker{md, wp}, module_{module}
  {
    if (wp.scheduleID_ == ScheduleID::first()) {
      // We only want to register the products (and any shared
      // resources) once, not once for every schedule...
      module_->registerProducts(wp.producedProducts_, md);
      wp.resources_.registerSharedResources(module_->sharedResources());
    } else {
      // ...but we need to fill product descriptions for each module
      // copy.
      module_->fillDescriptions(md);
    }
    ci_->outputModuleInitiated(
      label(),
      fhicl::ParameterSetRegistry::get(description().parameterSetID()));
  }

  hep::concurrency::SerialTaskQueueChain*
  OutputWorker::doSerialTaskQueueChain() const
  {
    return module_->serialTaskQueueChain();
  }

  void
  OutputWorker::doBeginJob(detail::SharedResources const& resources)
  {
    module_->doBeginJob(resources);
  }

  void
  OutputWorker::doEndJob()
  {
    module_->doEndJob();
  }

  void
  OutputWorker::doRespondToOpenInputFile(FileBlock const& fb)
  {
    module_->doRespondToOpenInputFile(fb);
  }

  void
  OutputWorker::doRespondToCloseInputFile(FileBlock const& fb)
  {
    module_->doRespondToCloseInputFile(fb);
  }

  void
  OutputWorker::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    module_->doRespondToOpenOutputFiles(fb);
  }

  void
  OutputWorker::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    module_->doRespondToCloseOutputFiles(fb);
  }

  bool
  OutputWorker::doBegin(RunPrincipal& rp, ModuleContext const& mc)
  {
    return module_->doBeginRun(rp, mc);
  }

  bool
  OutputWorker::doEnd(RunPrincipal& rp, ModuleContext const& mc)
  {
    return module_->doEndRun(rp, mc);
  }

  bool
  OutputWorker::doBegin(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    return module_->doBeginSubRun(srp, mc);
  }

  bool
  OutputWorker::doEnd(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    return module_->doEndSubRun(srp, mc);
  }

  bool
  OutputWorker::doProcess(EventPrincipal& ep, ModuleContext const& mc)
  {
    // Note, only filters ever return false, and when they do it means
    // they have rejected.
    return module_->doEvent(
      ep, mc, counts_run_, counts_passed_, counts_failed_);
  }

  std::string const&
  OutputWorker::lastClosedFileName() const
  {
    return module_->lastClosedFileName();
  }

  bool
  OutputWorker::closeFile()
  {
    bool const didCloseFile{module_->doCloseFile()};
    if (didCloseFile) {
      ci_->outputFileClosed(description().moduleLabel(), lastClosedFileName());
    }
    return didCloseFile;
  }

  void
  OutputWorker::incrementInputFileNumber()
  {
    return module_->incrementInputFileNumber();
  }

  bool
  OutputWorker::requestsToCloseFile() const
  {
    return module_->requestsToCloseFile();
  }

  bool
  OutputWorker::openFile(FileBlock const& fb)
  {
    bool const didOpenFile{module_->doOpenFile(fb)};
    if (didOpenFile) {
      ci_->outputFileOpened(description().moduleLabel());
    }
    return didOpenFile;
  }

  void
  OutputWorker::writeRun(RunPrincipal& rp)
  {
    module_->doWriteRun(rp);
  }

  void
  OutputWorker::writeSubRun(SubRunPrincipal& srp)
  {
    module_->doWriteSubRun(srp);
  }

  void
  OutputWorker::writeEvent(EventPrincipal& ep, ModuleContext const& mc)
  {
    module_->doWriteEvent(ep, mc);
  }

  void
  OutputWorker::setRunAuxiliaryRangeSetID(RangeSet const& rs)
  {
    module_->doSetRunAuxiliaryRangeSetID(rs);
  }

  void
  OutputWorker::setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
  {
    module_->doSetSubRunAuxiliaryRangeSetID(rs);
  }

  bool
  OutputWorker::fileIsOpen() const
  {
    return module_->fileIsOpen();
  }

  void
  OutputWorker::setFileStatus(OutputFileStatus const ofs)
  {
    return module_->setFileStatus(ofs);
  }

  bool
  OutputWorker::limitReached() const
  {
    return module_->limitReached();
  }

  void
  OutputWorker::configure(OutputModuleDescription const& desc)
  {
    module_->configure(desc);
  }

  void
  OutputWorker::selectProducts(ProductTables const& tables)
  {
    module_->selectProducts(tables);
  }

  Granularity
  OutputWorker::fileGranularity() const
  {
    return module_->fileGranularity();
  }

} // namespace art
