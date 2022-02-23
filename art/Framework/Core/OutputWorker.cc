#include "art/Framework/Core/OutputWorker.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Utilities/OutputFileInfo.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art {

  Worker*
  OutputWorker::makeWorker(std::shared_ptr<ModuleBase> mod,
                           WorkerParams const& wp)
  {
    return new OutputWorker{std::dynamic_pointer_cast<OutputModule>(mod), wp};
  }

  OutputWorker::~OutputWorker() = default;

  // This is called directly by the make_worker function created
  // by the DEFINE_ART_MODULE macro.
  OutputWorker::OutputWorker(std::shared_ptr<OutputModule> module,
                             WorkerParams const& wp)
    : Worker{module->moduleDescription(), wp}
    , module_{module}
    , actReg_{wp.actReg_}
  {
    if (wp.scheduleID_ == ScheduleID::first()) {
      // We only want to register the products (and any shared
      // resources) once, not once for every schedule)
      module_->registerProducts(wp.producedProducts_);
      wp.resources_.registerSharedResources(module_->sharedResources());
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

  void
  OutputWorker::doBegin(RunPrincipal& rp, ModuleContext const& mc)
  {
    module_->doBeginRun(rp, mc);
  }

  void
  OutputWorker::doEnd(RunPrincipal& rp, ModuleContext const& mc)
  {
    module_->doEndRun(rp, mc);
  }

  void
  OutputWorker::doBegin(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    module_->doBeginSubRun(srp, mc);
  }

  void
  OutputWorker::doEnd(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    module_->doEndSubRun(srp, mc);
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

  void
  OutputWorker::closeFile()
  {
    actReg_.sPreCloseOutputFile.invoke(label());
    if (module_->doCloseFile()) {
      ci_->outputFileClosed(label(), lastClosedFileName());
    }
    actReg_.sPostCloseOutputFile.invoke(
      OutputFileInfo{label(), lastClosedFileName()});
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

  void
  OutputWorker::openFile(FileBlock const& fb)
  {
    if (module_->doOpenFile(fb)) {
      ci_->outputFileOpened(label());
      actReg_.sPostOpenOutputFile.invoke(label());
    }
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
  OutputWorker::writeEvent(EventPrincipal& ep, PathContext const& pc)
  {
    ModuleContext const mc{pc, description()};
    actReg_.sPreWriteEvent.invoke(mc);
    module_->doWriteEvent(ep, mc);
    actReg_.sPostWriteEvent.invoke(mc);
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
