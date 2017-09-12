#ifndef art_Framework_Core_OutputWorker_h
#define art_Framework_Core_OutputWorker_h
// vim: set sw=2 expandtab :

//  The OutputModule as the schedule sees it.  The job of
//  this object is to call the output module.
//
//  According to our current definition, a single output module can only
//  appear in one worker.

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "canvas/Persistency/Provenance/ProductList.h"

#include <memory>

namespace art {

class RangeSet;

class OutputWorker : public WorkerT<OutputModule> {

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~OutputWorker();

  // This is called directly by the make_worker function created
  // by the DEFINE_ART_MODULE macro.
  OutputWorker(OutputModule* mod, ModuleDescription const&, WorkerParams const&);

public:

  std::string const&
  lastClosedFileName() const;

  void
  closeFile();

  bool
  fileIsOpen() const;

  void
  incrementInputFileNumber();

  bool
  requestsToCloseFile() const;

  bool
  wantAllEvents() const;

  void
  openFile(FileBlock const& fb);

  void
  writeRun(RunPrincipal& rp);

  void
  writeSubRun(SubRunPrincipal& srp);

  void
  writeEvent(EventPrincipal& ep);

  void
  setRunAuxiliaryRangeSetID(RangeSet const&);

  void
  setSubRunAuxiliaryRangeSetID(RangeSet const&);

  bool
  limitReached() const;

  void
  setFileStatus(OutputFileStatus);

  void
  configure(OutputModuleDescription const& desc);

  Granularity
  fileGranularity() const;

  virtual
  void
  selectProducts(ProductList const&);

private:

  ServiceHandle<CatalogInterface>
  ci_{};

  Granularity
  fileGranularity_{Granularity::Unset};

};

} // namespace art

#endif /* art_Framework_Core_OutputWorker_h */

// Local Variables:
// mode: c++
// End:
