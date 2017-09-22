#ifndef art_Framework_Core_OutputModule_h
#define art_Framework_Core_OutputModule_h
// vim: set sw=2 expandtab :

//
//  The base class of all modules that write Events to an output stream.
//

#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/FileCatalogMetadataPlugin.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/detail/parse_path_spec.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Persistency/Provenance/Selections.h"
#include "canvas/Persistency/Provenance/BranchChildren.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ParentageID.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "cetlib/BasicPluginFactory.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalTable.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/TableFragment.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace art {

class ResultsPrincipal;

class OutputModule : public EventObserverBase {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;
  friend class OutputWorker;

public: // TYPES

  // The module macros need these two.
  using ModuleType = OutputModule;
  using WorkerType = OutputWorker;

private: // TYPES

  using PluginCollection_t = std::vector<std::unique_ptr<FileCatalogMetadataPlugin>>;

public: // CONFIGURATION

  struct Config {

    struct KeysToIgnore {

      std::set<std::string>
      operator()()
      {
        return {"module_label", "streamName", "FCMDPlugins"};
      }

      static
      auto
      get()
      {
        return KeysToIgnore{}();
      }

    };

    fhicl::Atom<std::string>
    moduleType{fhicl::Name("module_type")};

    fhicl::TableFragment<EventObserverBase::EOConfig>
    eoFragment;

    fhicl::Sequence<std::string>
    outputCommands{fhicl::Name("outputCommands"), std::vector<std::string>{"keep *"}};

    fhicl::Atom<std::string>
    fileName{fhicl::Name("fileName"), ""};

    fhicl::Atom<std::string>
    dataTier{fhicl::Name("dataTier"), ""};

    fhicl::Atom<std::string>
    streamName{fhicl::Name("streamName"), ""};

  };

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~OutputModule() noexcept;

  explicit
  OutputModule(fhicl::ParameterSet const& pset);

  explicit
  OutputModule(fhicl::TableFragment<Config> const& pset, fhicl::ParameterSet const& containing_pset);

  OutputModule(OutputModule const&) = delete;

  OutputModule(OutputModule&&) = delete;

  OutputModule&
  operator=(OutputModule const&) = delete;

  OutputModule&
  operator=(OutputModule&&) = delete;

public: // MEMBER FUNCTIONS

  // Accessor for maximum number of events to be written.
  // -1 is used for unlimited.
  int
  maxEvents() const;

  // Accessor for remaining number of events to be written.
  // -1 is used for unlimited.
  int
  remainingEvents() const;

  bool
  fileIsOpen() const;

  OutputFileStatus
  fileStatus() const;

  // Name of output file (may be overridden if default implementation is
  // not appropriate).
  virtual
  std::string const&
  lastClosedFileName() const;

  SelectionsArray const&
  keptProducts() const;

  std::array<bool, NumBranchTypes> const&
  hasNewlyDroppedBranch() const;

  void
  selectProducts(ProductList const&);

  void
  doSelectProducts(ProductList const&);

  void registerProducts(MasterProductRegistry&,
                        ProductDescriptions&,
                        ModuleDescription const&);
  BranchChildren const&
  branchChildren() const;

protected:

  // Called to register products if necessary.
  virtual
  void
  doRegisterProducts(MasterProductRegistry&,
                     ProductDescriptions&,
                     ModuleDescription const&);

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  configure(OutputModuleDescription const& desc);

  virtual
  void
  doBeginJob();

  // Called after selectProducts() has done its work.
  virtual
  void
  postSelectProducts();

  void
  doEndJob();

  void
  doRespondToOpenInputFile(FileBlock const& fb);

  void
  doRespondToCloseInputFile(FileBlock const& fb);

  void
  doRespondToOpenOutputFiles(FileBlock const& fb);

  void
  doRespondToCloseOutputFiles(FileBlock const& fb);

  bool
  doBeginRun(RunPrincipal& rp, CurrentProcessingContext const* cpc);

  bool
  doEndRun(RunPrincipal& rp, CurrentProcessingContext const* cpc);

  bool
  doBeginSubRun(SubRunPrincipal& srp, CurrentProcessingContext const* cpc);

  bool
  doEndSubRun(SubRunPrincipal& srp, CurrentProcessingContext const* cpc);

  bool
  doEvent(EventPrincipal& ep, int streamIndex, CurrentProcessingContext const* cpc,
          std::atomic<std::size_t>& counts_run,
          std::atomic<std::size_t>& counts_passed,
          std::atomic<std::size_t>& counts_failed);

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  doWriteRun(RunPrincipal& rp);

  void
  doWriteSubRun(SubRunPrincipal& srp);

  void
  doWriteEvent(EventPrincipal& ep);

  void
  doSetRunAuxiliaryRangeSetID(RangeSet const&);

  void
  doSetSubRunAuxiliaryRangeSetID(RangeSet const&);

  void
  doOpenFile(FileBlock const& fb);

protected: // MEMBER FUNCTIONS -- Implementation API, intended to be provided by derived classes.


  std::string
  workerType() const;

  // Tell the OutputModule that it must end the current file.
  void
  doCloseFile();

  // Do the end-of-file tasks; this is only called internally, after
  // the appropriate tests have been done.
  void
  reallyCloseFile();

  virtual
  void
  incrementInputFileNumber();

  // Ask the OutputModule if we should end the current file.
  // N.B. The default file granularity is 'Unset', which means that
  //      even if an output module requests to close its file, the
  //      file will not switch.  To ensure that a file switch requires
  //      where desired, the author of the output module MUST provide
  //      an override.  It would be desirable to check if both
  //      requestsToCloseFile() and fileGranularity() could be checked
  //      at compile time.  However, such a check would require an
  //      interface change.
  virtual
  bool
  requestsToCloseFile() const;

  virtual
  Granularity
  fileGranularity() const;

  virtual
  void
  setFileStatus(OutputFileStatus);

  virtual
  void
  beginJob();

  virtual
  void
  endJob();

  virtual
  void
  beginRun(RunPrincipal&);

  virtual
  void
  endRun(RunPrincipal&);

  virtual
  void
  writeRun(RunPrincipal& r) = 0;

  virtual
  void
  setRunAuxiliaryRangeSetID(RangeSet const&);

  virtual
  void
  beginSubRun(SubRunPrincipal&);

  virtual
  void
  endSubRun(SubRunPrincipal&);

  virtual
  void
  writeSubRun(SubRunPrincipal& sr) = 0;

  virtual
  void
  setSubRunAuxiliaryRangeSetID(RangeSet const&);

  virtual
  void
  event(EventPrincipal&);

  virtual
  void
  event_in_stream(EventPrincipal&, int streamIndex);

  virtual
  void
  write(EventPrincipal& e) = 0;

  virtual
  void
  openFile(FileBlock const&);

  virtual
  void
  respondToOpenInputFile(FileBlock const&);

  virtual
  void
  readResults(ResultsPrincipal const& resp);

  virtual
  void
  respondToCloseInputFile(FileBlock const&);

  virtual
  void
  respondToOpenOutputFiles(FileBlock const&);

  virtual
  void
  respondToCloseOutputFiles(FileBlock const&);

  virtual
  bool
  isFileOpen() const;

  void
  updateBranchParents(EventPrincipal& ep);

  void
  fillDependencyGraph();

  bool
  limitReached() const;

  // The following member functions are part of the Template Method
  // pattern, used for implementing doCloseFile() and maybeEndFile().

  virtual
  void
  startEndFile();

  virtual
  void
  writeFileFormatVersion();

  virtual
  void
  writeFileIdentifier();

  virtual
  void
  writeFileIndex();

  virtual
  void
  writeEventHistory();

  virtual
  void
  writeProcessConfigurationRegistry();

  virtual
  void
  writeProcessHistoryRegistry();

  virtual
  void
  writeParameterSetRegistry();

  virtual
  void
  writeBranchIDListRegistry();

  virtual
  void
  writeParentageRegistry();

  virtual
  void
  writeProductDescriptionRegistry();

  void
  writeFileCatalogMetadata();

  virtual
  void
  doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                             FileCatalogMetadata::collection_type const& ssmd);

  virtual
  void
  writeProductDependencies();

  virtual
  void
  finishEndFile();

  PluginCollection_t
  makePlugins_(fhicl::ParameterSet const& top_pset);

// FIXME: Make these private data members.

protected: // MEMBER DATA -- For derived classes

  // TODO: Give OutputModule an interface (protected?) that supplies
  // client code with the needed functionality *without* giving away
  // implementation details ... don't just return a reference to
  // keptProducts_, because we are looking to have the flexibility to
  // change the implementation of keptProducts_ without modifying
  // clients. When this change is made, we'll have a one-time-only
  // task of modifying clients (classes derived from OutputModule) to
  // use the newly-introduced interface.  TODO: Consider using shared
  // pointers here?
  //
  // keptProducts_ are BranchDescription objects OWNED BY VALUE
  // describing the branches we are to write.

  SelectionsArray
  keptProducts_{{}};

  std::array<bool, NumBranchTypes>
  hasNewlyDroppedBranch_{{false}};

  GroupSelectorRules
  groupSelectorRules_;

  int
  maxEvents_{-1};

  int
  remainingEvents_{maxEvents_};

  using BranchParents = std::map<ProductID, std::set<ParentageID>>;

  std::map<ProductID, std::set<ParentageID>>
  branchParents_{};

  BranchChildren
  branchChildren_{};

  std::string
  configuredFileName_;

  std::string
  dataTier_;

  std::string
  streamName_;

  ServiceHandle<CatalogInterface>
  ci_{};

  cet::BasicPluginFactory
  pluginFactory_{};

  // For diagnostics.
  std::vector<std::string>
  pluginNames_ {};

  PluginCollection_t
  plugins_;

};

namespace one {

class OutputModule : public art::OutputModule {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;
  friend class OutputWorker;

public: // TYPES

  // The module macros need these two.
  using ModuleType = OutputModule;
  using WorkerType = OutputWorker;

private: // TYPES

  using PluginCollection_t = std::vector<std::unique_ptr<FileCatalogMetadataPlugin>>;

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~OutputModule() noexcept;

  explicit
  OutputModule(fhicl::ParameterSet const& pset);

  explicit
  OutputModule(fhicl::TableFragment<Config> const& pset, fhicl::ParameterSet const& containing_pset);

  OutputModule(OutputModule const&) = delete;

  OutputModule(OutputModule&&) = delete;

  OutputModule&
  operator=(OutputModule const&) = delete;

  OutputModule&
  operator=(OutputModule&&) = delete;

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  doBeginJob() override;

};

} // namespace one

namespace stream {

class OutputModule : public art::OutputModule {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;
  friend class OutputWorker;

public: // TYPES

  // The module macros need these two.
  using ModuleType = OutputModule;
  using WorkerType = OutputWorker;

private: // TYPES

  using PluginCollection_t = std::vector<std::unique_ptr<FileCatalogMetadataPlugin>>;

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~OutputModule() noexcept;

  explicit
  OutputModule(fhicl::ParameterSet const& pset);

  explicit
  OutputModule(fhicl::TableFragment<Config> const& pset, fhicl::ParameterSet const& containing_pset);

  OutputModule(OutputModule const&) = delete;

  OutputModule(OutputModule&&) = delete;

  OutputModule&
  operator=(OutputModule const&) = delete;

  OutputModule&
  operator=(OutputModule&&) = delete;

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  doBeginJob() override;

};

} // namespace stream

namespace global {

class OutputModule : public art::OutputModule {

  // Allow the WorkerT<T> ctor to call setModuleDescription() and workerType().
  template <typename T> friend class WorkerT;
  friend class OutputWorker;

public: // TYPES

  // The module macros need these two.
  using ModuleType = OutputModule;
  using WorkerType = OutputWorker;

private: // TYPES

  using PluginCollection_t = std::vector<std::unique_ptr<FileCatalogMetadataPlugin>>;

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~OutputModule() noexcept;

  explicit
  OutputModule(fhicl::ParameterSet const& pset);

  explicit
  OutputModule(fhicl::TableFragment<Config> const& pset, fhicl::ParameterSet const& containing_pset);

  OutputModule(OutputModule const&) = delete;

  OutputModule(OutputModule&&) = delete;

  OutputModule&
  operator=(OutputModule const&) = delete;

  OutputModule&
  operator=(OutputModule&&) = delete;

private: // MEMBER FUNCTIONS -- API required by EventProcessor, Schedule, and EndPathExecutor

  void
  doBeginJob() override;

};

} // namespace global

} // namespace art

#endif /* art_Framework_Core_OutputModule_h */

// Local Variables:
// mode: c++
// End:
