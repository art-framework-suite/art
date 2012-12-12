
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
    ci_(),
    fileName_()
  {
    ci_->outputModuleInitiated(description().moduleLabel(),
                               fhicl::ParameterSetRegistry::get(description().parameterSetID()));
  }

  OutputWorker::~OutputWorker() {
  }

  void
  OutputWorker::closeFile() {
    module().doCloseFile();
    if (!fileName_.empty()) {
      ci_->outputFileClosed(description().moduleLabel(), fileName_);
      fileName_ = std::string();
    }
  }

  bool
  OutputWorker::shouldWeCloseFile() const {
    return module().shouldWeCloseFile();
  }

  void
  OutputWorker::openNewFileIfNeeded() {
    module().maybeOpenFile();
  }

  void
  OutputWorker::openFile(FileBlock const& fb) {
    // FIXME: FileBlock abstraction is currently broken! When it is
    // fixed, we should get the correct filename (or generate it here,
    // even), since the FileBlock currently contains only the details
    // for the input file.
    //
    //    fileName_ = fb.fileName();
    fhicl::ParameterSetRegistry::
      get(description().parameterSetID()).get_if_present("fileName", fileName_);
    module().doOpenFile(fb);
    if (!fileName_.empty()) {
      ci_->outputFileOpened(description().moduleLabel());
    }
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
}
