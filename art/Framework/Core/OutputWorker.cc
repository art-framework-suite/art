
/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Core/WorkerParams.h"
#include "art/Framework/Core/OutputWorker.h"

namespace art {
  OutputWorker::OutputWorker(std::auto_ptr<OutputModule> mod,
			     ModuleDescription const& md,
			     WorkerParams const& wp):
      WorkerT<OutputModule>(mod, md, wp)
  {
  }

  OutputWorker::~OutputWorker() {
  }

  void
  OutputWorker::closeFile() {
    module().doCloseFile();
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
    module().doOpenFile(fb);
  }

  void
  OutputWorker::writeRun(RunPrincipal const& rp) {
    module().doWriteRun(rp);
  }

  void
  OutputWorker::writeSubRun(SubRunPrincipal const& lbp) {
    module().doWriteSubRun(lbp);
  }

  bool OutputWorker::wantAllEvents() const {return module().wantAllEvents();}

  bool OutputWorker::limitReached() const {return module().limitReached();}

  void OutputWorker::configure(OutputModuleDescription const& desc) {module().configure(desc);}
}
