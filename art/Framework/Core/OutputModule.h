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

#include "fhiclcpp/Atom.h"
#include "fhiclcpp/Sequence.h"
#include "fhiclcpp/ParameterSet.h"

#include <array>
#include <memory>
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

  // Configuration
  struct baseConfig {
    fhicl::Atom<std::string> moduleType { fhicl::Key("module_type") };
    fhicl::Table<EventObserver::EOConfig> eoConfig {
      fhicl::Key("SelectEvents"),
        fhicl::Comment("The 'SelectEvents' table below is optional")
        };
    fhicl::Sequence<std::string> outputCommands { fhicl::Key("outputCommands"), fhicl::Sequence<std::string>{ "keep *" } };
    fhicl::Atom<std::string> fileName   { fhicl::Key("fileName"), "" };
    fhicl::Atom<std::string> dataTier   { fhicl::Key("dataTier"), "" };
    fhicl::Atom<std::string> streamName { fhicl::Key("streamName"), "" };
  };

  template <typename T>
  struct fullConfig : baseConfig, T {};

  template < typename userConfig >
  class Table : public fhicl::Table<fullConfig<userConfig>> {
  public:

    Table(){}

    Table( fhicl::ParameterSet const& pset ) : Table()
      {
        std::set<std::string> const keys_to_ignore = { "module_label",
                                                       "streamName",
                                                       "FCMDPlugins",
                                                       "fastCloning" }; // From RootOuput (shouldn't be here")
        this->validate_ParameterSet( pset, keys_to_ignore );
        this->set_PSet( pset );
      }

  };

  template <typename Config>
  explicit OutputModule( Table<Config> const & pset);

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

  void selectProducts(FileBlock const&);

protected:
  // The returned pointer will be null unless the this is currently
  // executing its event loop function ('write').
  CurrentProcessingContext const * currentContext() const;

  ModuleDescription const & description() const;

  // Called before selectProducts() has done its work.
  virtual void preSelectProducts(FileBlock const &);

  // Called after selectProducts() has done its work.
  virtual void postSelectProducts(FileBlock const &);

private:

  SelectionsArray keptProducts_;
  std::array<bool, NumBranchTypes> hasNewlyDroppedBranch_;
  GroupSelectorRules groupSelectorRules_;
  GroupSelector groupSelector_;
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
  std::vector<std::string> pluginNames_; // For diagnostics.

  typedef std::vector<std::unique_ptr<FileCatalogMetadataPlugin> >
  PluginCollection_t;
  PluginCollection_t plugins_;

  //------------------------------------------------------------------
  // private member functions
  //------------------------------------------------------------------
  void configure(OutputModuleDescription const & desc);
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

  template <typename Config>
  PluginCollection_t makePlugins_(Table<Config> const& config);
};  // OutputModule

#ifndef __GCCXML__

template <typename Config>
auto
art::OutputModule::
makePlugins_(art::OutputModule::Table<Config> const & config)
  -> PluginCollection_t
{
  fhicl::ParameterSet const& top_pset = config.get_PSet();
  auto const psets = top_pset.get<std::vector<fhicl::ParameterSet>>("FCMDPlugins", {} );
  PluginCollection_t result;
  result.reserve(psets.size());
  size_t count = 0;
  try {
    for (auto const & pset : psets) {
      pluginNames_.emplace_back(pset.get<std::string>("plugin_type"));
      auto const & libspec = pluginNames_.back();
      auto const pluginType = pluginFactory_.pluginType(libspec);
      if (pluginType == cet::PluginTypeDeducer<FileCatalogMetadataPlugin>::value) {
        result.emplace_back(pluginFactory_.
                            makePlugin<std::unique_ptr<FileCatalogMetadataPlugin>,
                            fhicl::ParameterSet const &>(libspec, pset));
      } else {
        throw Exception(errors::Configuration, "OutputModule: ")
          << "unrecognized plugin type "
          << pluginType
          << ".\n";
      }
      ++count;
    }
  }
  catch (cet::exception & e) {
    throw Exception(errors::Configuration, "OutputModule: ", e)
      << "Exception caught while processing FCMDPlugins["
      << count
      << "] in module "
      << description().moduleLabel()
      << ".\n";
  }
  return result;
}


template <typename Config>
art::OutputModule::
OutputModule(art::OutputModule::Table<Config> const & config)
  :
  EventObserver(config.get_PSet()),
  keptProducts_(),
  hasNewlyDroppedBranch_(),
  groupSelectorRules_(config().outputCommands(), "outputCommands", "OutputModule"),
  groupSelector_(),
  maxEvents_(-1),
  remainingEvents_(maxEvents_),
  moduleDescription_(),
  current_context_(0),
  branchParents_(),
  branchChildren_(),
  // FIXME:
  //   For the next data member, qualifying 'fileName' is a necessity
  //   since some user-provided structs inlude 'fileName'
  //
  //     struct Config : OutputModuleConfig, UserConfig {};
  //
  //   Both OutputModuleConfig and RootOutputConfig include 'fileName'
  //   members, creating a lookup ambiguity.  Since only one entity
  //   exists in the ParameterSet, both with reference the same value.
  configuredFileName_(config().OutputModule::baseConfig::fileName()),
  dataTier_(config().dataTier()),
  streamName_(config().streamName()),
  ci_(),
  pluginFactory_(),
  pluginNames_(),
  plugins_(makePlugins_(config))
{
  hasNewlyDroppedBranch_.fill(false);
}

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
bool
art::OutputModule::
selected(BranchDescription const & desc) const
{
  return groupSelector_.selected(desc);
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
