#ifndef art_Framework_Core_OutputModule_h
#define art_Framework_Core_OutputModule_h

// ======================================================================
//
// OutputModule - The base class of all "modules" that write Events to an
//                output stream.
//
// ======================================================================

#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/FileCatalogMetadataPlugin.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Principal/Consumer.h"
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

#include <array>
#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {
  class OutputModule;
  class ResultsPrincipal;
}

class art::OutputModule : public EventObserverBase, public Consumer {
public:
  OutputModule(OutputModule const&) = delete;
  OutputModule& operator=(OutputModule const&) = delete;

  template <typename T>
  friend class WorkerT;
  friend class OutputWorker;
  using ModuleType = OutputModule;
  using WorkerType = OutputWorker;

  // Configuration
  struct Config {
    struct KeysToIgnore {
      std::set<std::string>
      operator()()
      {
        return {"module_label", "streamName", "FCMDPlugins"};
      }
      static auto
      get()
      {
        return KeysToIgnore{}();
      }
    };

    fhicl::Atom<std::string> moduleType{fhicl::Name("module_type")};
    fhicl::TableFragment<EventObserverBase::EOConfig> eoFragment;
    fhicl::Sequence<std::string> outputCommands{
      fhicl::Name("outputCommands"),
      std::vector<std::string>{"keep *"}};
    fhicl::Atom<std::string> fileName{fhicl::Name("fileName"), ""};
    fhicl::Atom<std::string> dataTier{fhicl::Name("dataTier"), ""};
    fhicl::Atom<std::string> streamName{fhicl::Name("streamName"), ""};
  };

  explicit OutputModule(fhicl::TableFragment<Config> const& pset,
                        fhicl::ParameterSet const& containing_pset);
  explicit OutputModule(fhicl::ParameterSet const& pset);

  virtual ~OutputModule() noexcept = default;

  // Accessor for maximum number of events to be written.
  // -1 is used for unlimited.
  int maxEvents() const;

  // Accessor for remaining number of events to be written.
  // -1 is used for unlimited.
  int remainingEvents() const;

  bool
  fileIsOpen() const
  {
    return isFileOpen();
  }
  OutputFileStatus fileStatus() const;

  // Name of output file (may be overridden if default implementation is
  // not appropriate).
  virtual std::string const& lastClosedFileName() const;

  bool selected(BranchDescription const&) const;
  SelectionsArray const& keptProducts() const;
  std::array<bool, NumBranchTypes> const& hasNewlyDroppedBranch() const;

  BranchChildren const& branchChildren() const;

  void selectProducts(ProductList const&);

  void registerProducts(MasterProductRegistry&,
                        ProductDescriptions&,
                        ModuleDescription const&);

protected:
  // The returned pointer will be null unless the this is currently
  // executing its event loop function ('write').
  CurrentProcessingContext const* currentContext() const;

  ModuleDescription const& description() const;

  // Called after selectProducts() has done its work.
  virtual void postSelectProducts();

  // Called to register products if necessary.
  virtual void doRegisterProducts(MasterProductRegistry&,
                                  ProductDescriptions&,
                                  ModuleDescription const&);

private:
  // TODO: Give OutputModule an interface (protected?) that supplies
  // client code with the needed functionality *without* giving away
  // implementation details ... don't just return a reference to
  // keptProducts_, because we are looking to have the flexibility to
  // change the implementation of keptProducts_ without modifying
  // clients. When this change is made, we'll have a one-time-only
  // task of modifying clients (classes derived from OutputModule) to
  // use the newly-introduced interface.  TODO: Consider using shared
  // pointers here?

  // keptProducts_ are pointers to the BranchDescription objects
  // describing the branches we are to write.
  //
  // We do not own the BranchDescriptions to which we point.

  SelectionsArray keptProducts_{{}}; // filled by aggregation
  std::array<bool, NumBranchTypes> hasNewlyDroppedBranch_{
    {false}}; // filled by aggregation
  GroupSelectorRules groupSelectorRules_;
  std::unique_ptr<GroupSelector const> groupSelector_{nullptr};
  int maxEvents_{-1};
  int remainingEvents_{maxEvents_};

  ModuleDescription moduleDescription_{};
  cet::exempt_ptr<CurrentProcessingContext const> current_context_{nullptr};

  using BranchParents = std::map<ProductID, std::set<ParentageID>>;
  BranchParents branchParents_{};

  BranchChildren branchChildren_{};

  std::string configuredFileName_;
  std::string dataTier_;
  std::string streamName_;
  ServiceHandle<CatalogInterface> ci_{};

  cet::BasicPluginFactory pluginFactory_{};
  std::vector<std::string> pluginNames_{}; // For diagnostics.

  using PluginCollection_t =
    std::vector<std::unique_ptr<FileCatalogMetadataPlugin>>;
  PluginCollection_t plugins_;

  //------------------------------------------------------------------
  // private member functions
  //------------------------------------------------------------------
  void configure(OutputModuleDescription const& desc);

  void doBeginJob();
  void doEndJob();
  bool doEvent(EventPrincipal const& ep,
               CurrentProcessingContext const* cpc,
               CountingStatistics&);
  bool doBeginRun(RunPrincipal const& rp, CurrentProcessingContext const* cpc);
  bool doEndRun(RunPrincipal const& rp, CurrentProcessingContext const* cpc);
  bool doBeginSubRun(SubRunPrincipal const& srp,
                     CurrentProcessingContext const* cpc);
  bool doEndSubRun(SubRunPrincipal const& srp,
                   CurrentProcessingContext const* cpc);
  void doWriteRun(RunPrincipal& rp);
  void doWriteSubRun(SubRunPrincipal& srp);
  void doWriteEvent(EventPrincipal& ep);
  void doSetRunAuxiliaryRangeSetID(RangeSet const&);
  void doSetSubRunAuxiliaryRangeSetID(RangeSet const&);
  void doOpenFile(FileBlock const& fb);
  void doRespondToOpenInputFile(FileBlock const& fb);
  void doRespondToCloseInputFile(FileBlock const& fb);
  void doRespondToOpenOutputFiles(FileBlock const& fb);
  void doRespondToCloseOutputFiles(FileBlock const& fb);
  void doSelectProducts(ProductList const&);

  std::string
  workerType() const
  {
    return "OutputWorker";
  }

  // Tell the OutputModule that it must end the current file.
  void doCloseFile();

  // Do the end-of-file tasks; this is only called internally, after
  // the appropriate tests have been done.
  void reallyCloseFile();

  virtual void
  incrementInputFileNumber()
  {}

  // Ask the OutputModule if we should end the current file.
  // N.B. The default file granularity is 'Unset', which means that
  //      even if an output module requests to close its file, the
  //      file will not switch.  To ensure that a file switch requires
  //      where desired, the author of the output module MUST provide
  //      an override.  It would be desirable to check if both
  //      requestsToCloseFile() and fileGranularity() could be checked
  //      at compile time.  However, such a check would require an
  //      interface change.
  virtual bool
  requestsToCloseFile() const
  {
    return false;
  }
  virtual Granularity
  fileGranularity() const
  {
    return Granularity::Unset;
  }
  virtual void setFileStatus(OutputFileStatus);

  virtual void beginJob();
  virtual void endJob();
  virtual void beginRun(RunPrincipal const&);
  virtual void endRun(RunPrincipal const&);
  virtual void writeRun(RunPrincipal& r) = 0;
  virtual void setRunAuxiliaryRangeSetID(RangeSet const&);
  virtual void beginSubRun(SubRunPrincipal const&);
  virtual void endSubRun(SubRunPrincipal const&);
  virtual void writeSubRun(SubRunPrincipal& sr) = 0;
  virtual void setSubRunAuxiliaryRangeSetID(RangeSet const&);
  virtual void event(EventPrincipal const&);
  virtual void write(EventPrincipal& e) = 0;

  virtual void openFile(FileBlock const&);
  virtual void respondToOpenInputFile(FileBlock const&);
  virtual void readResults(ResultsPrincipal const& resp);
  virtual void respondToCloseInputFile(FileBlock const&);
  virtual void respondToOpenOutputFiles(FileBlock const&);
  virtual void respondToCloseOutputFiles(FileBlock const&);

  virtual bool isFileOpen() const;

  void setModuleDescription(ModuleDescription const& md);

  void updateBranchParents(EventPrincipal const& ep);
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
  virtual void writeParentageRegistry();
  virtual void writeProductDescriptionRegistry();
  void writeFileCatalogMetadata();
  virtual void doWriteFileCatalogMetadata(
    FileCatalogMetadata::collection_type const& md,
    FileCatalogMetadata::collection_type const& ssmd);
  virtual void writeProductDependencies();
  virtual void writeBranchMapper();
  virtual void finishEndFile();

  PluginCollection_t makePlugins_(fhicl::ParameterSet const& top_pset);
}; // OutputModule

inline art::CurrentProcessingContext const*
art::OutputModule::currentContext() const
{
  return current_context_.get();
}

inline art::ModuleDescription const&
art::OutputModule::description() const
{
  return moduleDescription_;
}

inline int
art::OutputModule::maxEvents() const
{
  return maxEvents_;
}

inline int
art::OutputModule::remainingEvents() const
{
  return remainingEvents_;
}

inline bool
art::OutputModule::selected(BranchDescription const& pd) const
{
  assert(groupSelector_);
  return groupSelector_->selected(pd);
}

inline auto
art::OutputModule::keptProducts() const -> SelectionsArray const&
{
  return keptProducts_;
}

inline auto
art::OutputModule::hasNewlyDroppedBranch() const
  -> std::array<bool, NumBranchTypes> const&
{
  return hasNewlyDroppedBranch_;
}

inline art::BranchChildren const&
art::OutputModule::branchChildren() const
{
  return branchChildren_;
}

inline void
art::OutputModule::setModuleDescription(ModuleDescription const& md)
{
  moduleDescription_ = md;
}

inline bool
art::OutputModule::limitReached() const
{
  return remainingEvents_ == 0;
}

#endif /* art_Framework_Core_OutputModule_h */

// Local Variables:
// mode: c++
// End:
