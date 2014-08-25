#ifndef art_Framework_Core_OutputModule_h
#define art_Framework_Core_OutputModule_h

// ======================================================================
//
// OutputModule - The base class of all "modules" that write Events to an
//                output stream.
//
// ======================================================================

#include "art/Framework/Core/EventObserver.h"
#include "art/Framework/Core/FileCatalogMetadataPlugin.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "cetlib/BasicPluginFactory.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ParentageID.h"
#include "art/Persistency/Provenance/Selections.h"

#include "cpp0x/array"
#include "cpp0x/memory"
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {
  class OutputModule;
}

class art::OutputModule : public EventObserver {
public:
  OutputModule(OutputModule const &) = delete;
  OutputModule & operator=(OutputModule const &) = delete;

  template <typename T> friend class WorkerT;
  friend class OutputWorker;
  typedef OutputModule ModuleType;
  typedef OutputWorker WorkerType;

  explicit OutputModule(fhicl::ParameterSet const & pset);
  virtual ~OutputModule() = default;
  virtual void reconfigure(fhicl::ParameterSet const &);

  // Accessor for maximum number of events to be written.
  // -1 is used for unlimited.
  int maxEvents() const;

  // Accessor for remaining number of events to be written.
  // -1 is used for unlimited.
  int remainingEvents() const;

  // Name of output file (may be overridden if default implementation is
  // not appropriate).
  virtual std::string const & lastClosedFileName() const;

  bool selected(BranchDescription const & desc) const;

  SelectionsArray const & keptProducts() const;
  std::array<bool, NumBranchTypes> const & hasNewlyDroppedBranch() const;

  BranchChildren const & branchChildren() const;

protected:
  // The returned pointer will be null unless the this is currently
  // executing its event loop function ('write').
  CurrentProcessingContext const * currentContext() const;

  ModuleDescription const & description() const;

private:
  int maxEvents_;
  int remainingEvents_;

  // TODO: Give OutputModule
  // an interface (protected?) that supplies client code with the
  // needed functionality *without* giving away implementation
  // details ... don't just return a reference to keptProducts_, because
  // we are looking to have the flexibility to change the
  // implementation of keptProducts_ without modifying clients. When this
  // change is made, we'll have a one-time-only task of modifying
  // clients (classes derived from OutputModule) to use the
  // newly-introduced interface.
  // TODO: Consider using shared pointers here?

  // keptProducts_ are pointers to the BranchDescription objects describing
  // the branches we are to write.
  //
  // We do not own the BranchDescriptions to which we point.
  SelectionsArray keptProducts_;

  std::array<bool, NumBranchTypes> hasNewlyDroppedBranch_;

  GroupSelectorRules groupSelectorRules_;
  GroupSelector groupSelector_;
  ModuleDescription moduleDescription_;

  // We do not own the pointed-to CurrentProcessingContext.
  CurrentProcessingContext const * current_context_;

  typedef std::map<BranchID, std::set<ParentageID> > BranchParents;
  BranchParents branchParents_;

  BranchChildren branchChildren_;

  std::string configuredFileName_;
  std::string dataTier_;
  std::string streamName_;
  ServiceHandle<CatalogInterface> ci_;

  cet::BasicPluginFactory pluginFactory_;
  typedef std::vector<std::unique_ptr<FileCatalogMetadataPlugin> >
  PluginCollection_t;
  PluginCollection_t plugins_;

  //------------------------------------------------------------------
  // private member functions
  //------------------------------------------------------------------
  void configure(OutputModuleDescription const & desc);
  void selectProducts();
  void doBeginJob();
  void doEndJob();
  bool doEvent(EventPrincipal const & ep,
               CurrentProcessingContext const * cpc);
  bool doBeginRun(RunPrincipal const & rp,
                  CurrentProcessingContext const * cpc);
  bool doEndRun(RunPrincipal const & rp,
                CurrentProcessingContext const * cpc);
  bool doBeginSubRun(SubRunPrincipal const & srp,
                     CurrentProcessingContext const * cpc);
  bool doEndSubRun(SubRunPrincipal const & srp,
                   CurrentProcessingContext const * cpc);
  void doWriteRun(RunPrincipal const & rp);
  void doWriteSubRun(SubRunPrincipal const & srp);
  void doOpenFile(FileBlock const & fb);
  void doRespondToOpenInputFile(FileBlock const & fb);
  void doRespondToCloseInputFile(FileBlock const & fb);
  void doRespondToOpenOutputFiles(FileBlock const & fb);
  void doRespondToCloseOutputFiles(FileBlock const & fb);

  std::string workerType() const {return "OutputWorker";}

  // Tell the OutputModule that is must end the current file.
  void doCloseFile();

  // Do the end-of-file tasks; this is only called internally, after
  // the appropriate tests have been done.
  void reallyCloseFile();

  // Ask the OutputModule if we should end the current file.
  virtual bool shouldWeCloseFile() const {return false;}

  // Write the event.
  virtual void write(EventPrincipal const & e) = 0;

  virtual void beginJob();
  virtual void endJob();
  virtual void beginRun(RunPrincipal const &);
  virtual void endRun(RunPrincipal const &);
  virtual void writeRun(RunPrincipal const & r) = 0;
  virtual void beginSubRun(SubRunPrincipal const &);
  virtual void endSubRun(SubRunPrincipal const &);
  virtual void writeSubRun(SubRunPrincipal const & sr) = 0;
  virtual void openFile(FileBlock const &);
  virtual void respondToOpenInputFile(FileBlock const &);
  virtual void respondToCloseInputFile(FileBlock const &);
  virtual void respondToOpenOutputFiles(FileBlock const &);
  virtual void respondToCloseOutputFiles(FileBlock const &);

  virtual bool isFileOpen() const;

  void setModuleDescription(ModuleDescription const & md);

  void updateBranchParents(EventPrincipal const & ep);
  void fillDependencyGraph();

  bool limitReached() const;

  // The following member functions are part of the Template Method
  // pattern, used for implementing doCloseFile() and maybeEndFile().

  virtual void startEndFile();
  virtual void writeFileFormatVersion();
  virtual void writeFileIdentifier();
  virtual void writeFileIndex();
  virtual void writeEventHistory();
  virtual void writeProcessConfigurationRegistry();
  virtual void writeProcessHistoryRegistry();
  virtual void writeParameterSetRegistry();
  virtual void writeBranchIDListRegistry();
  virtual void writeParentageRegistry();
  virtual void writeProductDescriptionRegistry();
  void writeFileCatalogMetadata();
  virtual void
  doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const & md,
                             FileCatalogMetadata::collection_type const & ssmd);
  virtual void writeProductDependencies();
  virtual void writeBranchMapper();
  virtual void finishEndFile();

  PluginCollection_t makePlugins_(std::vector<fhicl::ParameterSet> const psets);
};  // OutputModule

#ifndef __GCCXML__
inline
int
art::OutputModule::
maxEvents() const
{
  return maxEvents_;
}

inline
int
art::OutputModule::
remainingEvents() const
{
  return remainingEvents_;
}

inline
auto
art::OutputModule::
keptProducts() const
-> SelectionsArray const &
{
  return keptProducts_;
}

inline
auto
art::OutputModule::
hasNewlyDroppedBranch() const
-> std::array<bool, NumBranchTypes> const &
{
  return hasNewlyDroppedBranch_;
}

inline
art::BranchChildren const &
art::OutputModule::
branchChildren() const
{
  return branchChildren_;
}

inline
void
art::OutputModule::
setModuleDescription(ModuleDescription const & md)
{
  moduleDescription_ = md;
}

inline
bool
art::OutputModule::
limitReached() const
{
  return remainingEvents_ == 0;
}


#endif /* _GCCXML__ */

#endif /* art_Framework_Core_OutputModule_h */

// Local Variables:
// mode: c++
// End:
