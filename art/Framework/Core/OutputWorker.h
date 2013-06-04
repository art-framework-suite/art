#ifndef art_Framework_Core_OutputWorker_h
#define art_Framework_Core_OutputWorker_h

/*----------------------------------------------------------------------

OutputWorker: The OutputModule as the schedule sees it.  The job of
this object is to call the output module.

According to our current definition, a single output module can only
appear in one worker.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "cpp0x/memory"

namespace art {

  class OutputWorker : public WorkerT<OutputModule> {
  public:
    OutputWorker(std::unique_ptr<OutputModule> && mod,
                 ModuleDescription const&,
                 WorkerParams const&);

    virtual ~OutputWorker();

    // Call closeFile() on the controlled OutputModule.
    void closeFile();

    // Call shouldWeCloseFile() on the controlled OutputModule.
    bool shouldWeCloseFile() const;

    bool wantAllEvents() const;

    void openFile(FileBlock const& fb);

    void writeRun(RunPrincipal const& rp);

    void writeSubRun(SubRunPrincipal const& srp);

    bool limitReached() const;

    void configure(OutputModuleDescription const& desc);

private:
    ServiceHandle<CatalogInterface> ci_;
    std::string fileName_; // Needed for CatalogInterface interactions.
  };

}

#endif /* art_Framework_Core_OutputWorker_h */

// Local Variables:
// mode: c++
// End:
