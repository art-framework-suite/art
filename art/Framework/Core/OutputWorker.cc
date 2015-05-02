
/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/OutputWorker.h"

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art {
  OutputWorker::OutputWorker(std::unique_ptr<OutputModule> && mod,
                             ModuleDescription const& md,
                             WorkerParams const& wp):
    WorkerT<OutputModule>(std::move(mod), md, wp),
    ci_()
  {
    ci_->outputModuleInitiated(label(),
                               fhicl::ParameterSetRegistry::get(description().parameterSetID()));
  }

  OutputWorker::~OutputWorker() {
  }

  std::string const &
  OutputWorker::
  lastClosedFileName() const
  {
    return module().lastClosedFileName();
  }

  void
  OutputWorker::closeFile() {
    module().doCloseFile();
    ci_->outputFileClosed(description().moduleLabel(), module().lastClosedFileName());
  }

  bool
  OutputWorker::shouldWeCloseFile() const {
    return module().shouldWeCloseFile();
  }

  void
  OutputWorker::openFile(FileBlock const& fb) {
    module().doOpenFile(fb);
    ci_->outputFileOpened(description().moduleLabel());
  }

  void
  OutputWorker::writeRun(RunPrincipal const& rp) {
    module().doWriteRun(rp);
  }

  void
  OutputWorker::writeSubRun(SubRunPrincipal const& srp) {
    module().doWriteSubRun(srp);
  }

  bool OutputWorker::wantAllEvents() const {return module().wantAllEvents();}

  bool OutputWorker::limitReached() const {return module().limitReached();}

  void OutputWorker::configure(OutputModuleDescription const& desc) {module().configure(desc);}

  void OutputWorker::selectProducts(FileBlock const& fb) { module().selectProducts(fb); }

}
