// vim: set sw=2 expandtab :

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Core/RPManager.h"
#include "art/Framework/Core/ResultsProducer.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/RootFileBlock.h"
#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"
#include "art/Framework/IO/Root/RootOutputFile.h"
#include "art/Framework/IO/Root/detail/rootOutputConfigurationTools.h"
#include "art/Framework/IO/detail/logFileAction.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/parent_path.h"
#include "art/Utilities/unique_filename.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Table.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

using namespace std;


namespace art {

class RootOutputFile;

class RootOutput final : public OutputModule {

public:

  static constexpr char const* default_tmpDir {"<parent-path-of-filename>"};

  struct Config {

    using Name = fhicl::Name;
    using Comment = fhicl::Comment;
    template <typename T> using Atom = fhicl::Atom<T>;
    template <typename T> using OptionalAtom = fhicl::OptionalAtom<T>;

    fhicl::TableFragment<OutputModule::Config> omConfig;
    Atom<string> catalog { Name("catalog"), "" };
    OptionalAtom<bool> dropAllEvents { Name("dropAllEvents") };
    Atom<bool> dropAllSubRuns { Name("dropAllSubRuns"), false };
    OptionalAtom<bool> fastCloning { Name("fastCloning") };
    Atom<string> tmpDir { Name("tmpDir"), default_tmpDir };
    Atom<int> compressionLevel { Name("compressionLevel"), 7 };
    Atom<int64_t> saveMemoryObjectThreshold { Name("saveMemoryObjectThreshold"), -1l };
    Atom<int64_t> treeMaxVirtualSize { Name("treeMaxVirtualSize"), -1 };
    Atom<int> splitLevel { Name("splitLevel"), 99 };
    Atom<int> basketSize { Name("basketSize"), 16384 };
    Atom<bool> dropMetaDataForDroppedData { Name("dropMetaDataForDroppedData"), false };
    Atom<string> dropMetaData { Name("dropMetaData"), "NONE" };
    Atom<bool> writeParameterSets { Name("writeParameterSets"), true };
    fhicl::Table<ClosingCriteria::Config> fileProperties { Name("fileProperties") };

    Config()
    {
      // Both RootOutput module and OutputModule use the "fileName"
      // FHiCL parameter.  However, whereas in OutputModule the
      // parameter has a default, for RootOutput the parameter should
      // not.  We therefore have to change the default flag setting
      // for 'OutputModule::Config::fileName'.
      using namespace fhicl::detail;
      ParameterBase* adjustFilename {const_cast<fhicl::Atom<std::string>*>(&omConfig().fileName)};
      adjustFilename->set_par_style(fhicl::par_style::REQUIRED);
    }

    struct KeysToIgnore {
      set<string> operator()() const
      {
        set<string> keys {OutputModule::Config::KeysToIgnore::get()};
        keys.insert("results");
        return keys;
      }
    };

  };

  using Parameters = fhicl::WrappedTable<Config, Config::KeysToIgnore>;

public:

  ~RootOutput();

  explicit RootOutput(Parameters const&);

  RootOutput(RootOutput const&) = delete;

  RootOutput(RootOutput&&) = delete;

  RootOutput&
  operator=(RootOutput const&) = delete;

  RootOutput&
  operator=(RootOutput&&) = delete;

public:

  void postSelectProducts() override;

  void beginJob() override;
  void endJob() override;

  void event(EventPrincipal&) override;

  void beginSubRun(SubRunPrincipal&) override;
  void endSubRun(SubRunPrincipal&) override;

  void beginRun(RunPrincipal&) override;
  void endRun(RunPrincipal&) override;

private:

  string const&
  lastClosedFileName() const override;

  Granularity
  fileGranularity() const override;

  void openFile(FileBlock const&) override;
  void respondToOpenInputFile(FileBlock const&) override;
  void readResults(ResultsPrincipal const& resp) override;
  void respondToCloseInputFile(FileBlock const&) override;
  void incrementInputFileNumber() override;
  void write(EventPrincipal&) override;
  void writeSubRun(SubRunPrincipal&) override;
  void writeRun(RunPrincipal&) override;
  void setSubRunAuxiliaryRangeSetID(RangeSet const&) override;
  void setRunAuxiliaryRangeSetID(RangeSet const&) override;
  bool isFileOpen() const override;
  void setFileStatus(OutputFileStatus) override;
  bool requestsToCloseFile() const override;
  void doOpenFile();
  void startEndFile() override;
  void writeFileFormatVersion() override;
  void writeFileIndex() override;
  void writeEventHistory() override;
  void writeProcessConfigurationRegistry() override;
  void writeProcessHistoryRegistry() override;
  void writeParameterSetRegistry() override;
  void writeProductDescriptionRegistry() override;
  void writeParentageRegistry() override;

  void
  doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                             FileCatalogMetadata::collection_type const& ssmd) override;

  void writeProductDependencies() override;
  void finishEndFile() override;

  void doRegisterProducts(MasterProductRegistry& mpr,
                          ProductDescriptions& productsToProduce,
                          ModuleDescription const& md) override;

private:

  string const catalog_;
  bool dropAllEvents_ {false};
  bool dropAllSubRuns_;
  string const moduleLabel_;
  int inputFileCount_ {0};
  unique_ptr<RootOutputFile> rootOutputFile_ {nullptr};
  FileStatsCollector fstats_;
  string const filePattern_;
  string tmpDir_;
  string lastClosedFileName_ {};

  // We keep this set of data members for the use of RootOutputFile.
  int const compressionLevel_;
  int64_t const saveMemoryObjectThreshold_;
  int64_t const treeMaxVirtualSize_;
  int const splitLevel_;
  int const basketSize_;
  DropMetaData dropMetaData_;
  bool dropMetaDataForDroppedData_;

  // We keep this for the use of RootOutputFile and we also use it
  // during file open to make some choices.
  bool fastCloningEnabled_{true};

  // Set false only for cases where we are guaranteed never to need
  // historical ParameterSet information in the downstream file
  // (e.g. mixing).
  bool writeParameterSets_;
  ClosingCriteria fileProperties_;

  // ResultsProducer management.
  ProductDescriptions productsToProduce_{};
  ProductTables producedResultsProducts_{ProductTables::invalid()};
  RPManager rpm_;

};

RootOutput::
~RootOutput()
{
}

RootOutput::
RootOutput(Parameters const& config)
  : OutputModule{config().omConfig, config.get_PSet()}
  , catalog_{config().catalog()}
  , dropAllSubRuns_{config().dropAllSubRuns()}
  , moduleLabel_{config.get_PSet().get<string>("module_label")}
  , fstats_{moduleLabel_, processName()}
  , filePattern_{config().omConfig().fileName()}
  , tmpDir_{config().tmpDir() == default_tmpDir ? parent_path(filePattern_) : config().tmpDir()}
  , compressionLevel_{config().compressionLevel()}
  , saveMemoryObjectThreshold_{config().saveMemoryObjectThreshold()}
  , treeMaxVirtualSize_{config().treeMaxVirtualSize()}
  , splitLevel_{config().splitLevel()}
  , basketSize_{config().basketSize()}
  , dropMetaData_{config().dropMetaData()}
  , dropMetaDataForDroppedData_{config().dropMetaDataForDroppedData()}
  , writeParameterSets_{config().writeParameterSets()}
  , fileProperties_{(detail::validateFileNamePattern(config.get_PSet().has_key(config().fileProperties.name()), filePattern_),
                    config().fileProperties())}
  , rpm_{config.get_PSet()}
{
  bool const dropAllEventsSet {config().dropAllEvents(dropAllEvents_)};
  dropAllEvents_ = detail::shouldDropEvents(dropAllEventsSet, dropAllEvents_, dropAllSubRuns_);
  // N.B. Any time file switching is enabled at a boundary other than
  //      InputFile, fastCloningEnabled_ ***MUST*** be deactivated.  This is
  //      to ensure that the Event tree from the InputFile is not
  //      accidentally cloned to the output file before the output
  //      module has seen the events that are going to be processed.
  bool const fastCloningSet {config().fastCloning(fastCloningEnabled_)};
  fastCloningEnabled_ = RootOutputFile::shouldFastClone(fastCloningSet, fastCloningEnabled_, wantAllEvents(), fileProperties_);
  if (!writeParameterSets_) {
    mf::LogWarning("PROVENANCE")
        << "Output module " << moduleLabel_ << " has parameter writeParameterSets set to false.\n"
        << "Parameter set provenance will not be available in subsequent jobs.\n"
        << "Check your experiment's policy on this issue to avoid future problems\n"
        << "with analysis reproducibility.\n";
  }
}

void
RootOutput::
openFile(FileBlock const& fb)
{
  // Note: The file block here refers to the currently open
  //       input file, so we can find out about the available
  //       products by looping over the branches of the input
  //       file data trees.
  if (!isFileOpen()) {
    doOpenFile();
    respondToOpenInputFile(fb);
  }
}

void
RootOutput::
postSelectProducts()
{
  if (isFileOpen()) {
    rootOutputFile_->selectProducts();
  }
}

void
RootOutput::
respondToOpenInputFile(FileBlock const& fb)
{
  ++inputFileCount_;
  if (!isFileOpen()) return;

  auto const* rfb = dynamic_cast<RootFileBlock const*>(&fb);

  bool fastCloneThisOne = fastCloningEnabled_ && rfb && (rfb->tree() != nullptr) &&
                          ((remainingEvents() < 0) ||
                           (remainingEvents() >= rfb->tree()->GetEntries()));
  if (fastCloningEnabled_ && !fastCloneThisOne) {
    mf::LogWarning("FastCloning")
        << "Fast cloning deactivated for this input file due to "
        << "empty event tree and/or event limits.";
  }
  if (fastCloneThisOne && !rfb->fastClonable()) {
    mf::LogWarning("FastCloning")
        << "Fast cloning deactivated for this input file due to "
        << "information in FileBlock.";
    fastCloneThisOne = false;
  }
  rootOutputFile_->beginInputFile(rfb, fastCloneThisOne);
  fstats_.recordInputFile(fb.fileName());
}

void
RootOutput::
readResults(ResultsPrincipal const& resp)
{
  rpm_.for_each_RPWorker([&resp](RPWorker& w) {
    w.rp().doReadResults(resp);
  });
}

void
RootOutput::
respondToCloseInputFile(FileBlock const& fb)
{
  if (isFileOpen()) {
    rootOutputFile_->respondToCloseInputFile(fb);
  }
}

void
RootOutput::
write(EventPrincipal& ep)
{
  if (dropAllEvents_) {
    return;
  }
  if (hasNewlyDroppedBranch()[InEvent]) {
    ep.addToProcessHistory();
  }
  rootOutputFile_->writeOne(ep);
  fstats_.recordEvent(ep.eventID());
}

void
RootOutput::
setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
{
  rootOutputFile_->setSubRunAuxiliaryRangeSetID(rs);
}

void
RootOutput::
writeSubRun(SubRunPrincipal& sr)
{
  if (dropAllSubRuns_) {
    return;
  }
  if (hasNewlyDroppedBranch()[InSubRun]) {
    sr.addToProcessHistory();
  }
  rootOutputFile_->writeSubRun(sr);
  fstats_.recordSubRun(sr.subRunID());
}

void
RootOutput::
setRunAuxiliaryRangeSetID(RangeSet const& rs)
{
  rootOutputFile_->setRunAuxiliaryRangeSetID(rs);
}

void
RootOutput::
writeRun(RunPrincipal& rp)
{
  if (hasNewlyDroppedBranch()[InRun]) {
    rp.addToProcessHistory();
  }
  rootOutputFile_->writeRun(rp);
  fstats_.recordRun(rp.runID());
}

void
art::RootOutput::startEndFile()
{
  auto resp = std::make_unique<ResultsPrincipal>(ResultsAuxiliary{},
                                                 moduleDescription().processConfiguration(),
                                                 nullptr);
  resp->setProducedProducts(producedResultsProducts_);
  if (ProductMetaData::instance().productProduced(InResults) ||
      hasNewlyDroppedBranch()[InResults]) {
    resp->addToProcessHistory();
  }
  rpm_.for_each_RPWorker([&resp](RPWorker& w) {
      w.rp().doWriteResults(*resp);
    });
  rootOutputFile_->writeResults(*resp);
}

void
RootOutput::
writeFileFormatVersion()
{
  rootOutputFile_->writeFileFormatVersion();
}

void
RootOutput::
writeFileIndex()
{
  rootOutputFile_->writeFileIndex();
}

void
RootOutput::
writeEventHistory()
{
  rootOutputFile_->writeEventHistory();
}

void
RootOutput::
writeProcessConfigurationRegistry()
{
  rootOutputFile_->writeProcessConfigurationRegistry();
}

void
RootOutput::
writeProcessHistoryRegistry()
{
  rootOutputFile_->writeProcessHistoryRegistry();
}

void
RootOutput::
writeParameterSetRegistry()
{
  if (writeParameterSets_) {
    rootOutputFile_->writeParameterSetRegistry();
  }
}

void
RootOutput::
writeProductDescriptionRegistry()
{
  rootOutputFile_->writeProductDescriptionRegistry();
}

void
RootOutput::
writeParentageRegistry()
{
  rootOutputFile_->writeParentageRegistry();
}

void
RootOutput::
doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                           FileCatalogMetadata::collection_type const& ssmd)
{
  rootOutputFile_->writeFileCatalogMetadata(fstats_, md, ssmd);
}

void
RootOutput::writeProductDependencies()
{
  rootOutputFile_->writeProductDependencies();
}

void
RootOutput::finishEndFile()
{
  string const currentFileName {rootOutputFile_->currentFileName()};
  rootOutputFile_->writeTTrees();
  rootOutputFile_.reset();
  fstats_.recordFileClose();
  lastClosedFileName_ = PostCloseFileRenamer{fstats_} .maybeRenameFile(currentFileName, filePattern_);
  detail::logFileAction("Closed output file ", lastClosedFileName_);
  rpm_.invoke(&ResultsProducer::doClear);
}

void
RootOutput::
doRegisterProducts(MasterProductRegistry& mpr,
                   ProductDescriptions& producedProducts,
                   ModuleDescription const& md)
{
  // Register Results products from ResultsProducers.
  rpm_.for_each_RPWorker([&mpr, &producedProducts, &md](RPWorker& w) {
    auto const& params = w.params();
    w.setModuleDescription(ModuleDescription{params.rpPSetID,
                           params.rpPluginType,
                           md.moduleLabel() + '#' + params.rpLabel,
                           static_cast<int>(ModuleThreadingType::LEGACY),
                           md.processConfiguration(),
                           ModuleDescription::invalidID()});
    w.rp().registerProducts(mpr, producedProducts, w.moduleDescription());
  });

  // Form product table for Results products.  We do this here so we
  // can appropriately set the product tables for the
  // ResultsPrincipal.
  productsToProduce_ = producedProducts;
  producedResultsProducts_ = ProductTables{productsToProduce_};
}

void
RootOutput::setFileStatus(OutputFileStatus const ofs)
{
  if (isFileOpen()) {
    rootOutputFile_->setFileStatus(ofs);
  }
}

bool
RootOutput::
isFileOpen() const
{
  return rootOutputFile_.get() != nullptr;
}

void
RootOutput::incrementInputFileNumber()
{
  if (isFileOpen()) {
    rootOutputFile_->incrementInputFileNumber();
  }
}

bool
RootOutput::
requestsToCloseFile() const
{
  return isFileOpen() ? rootOutputFile_->requestsToCloseFile() : false;
}

Granularity
RootOutput::
fileGranularity() const
{
  return fileProperties_.granularity();
}

void
RootOutput::
doOpenFile()
{
  if (inputFileCount_ == 0) {
    throw Exception(errors::LogicError)
        << "Attempt to open output file before input file. "
        << "Please report this to the core framework developers.\n";
  }
  auto filename = unique_filename(tmpDir_ + "/RootOutput");
  rootOutputFile_ = make_unique<RootOutputFile>(this,
                    filename,
                    fileProperties_,
                    compressionLevel_,
                    saveMemoryObjectThreshold_,
                    treeMaxVirtualSize_,
                    splitLevel_,
                    basketSize_,
                    dropMetaData_,
                    dropMetaDataForDroppedData_,
                    fastCloningEnabled_);
  fstats_.recordFileOpen();
  string msg = "Opened output file ";
  //msg += filename;
  msg += " with pattern ";
  detail::logFileAction(msg.c_str(), filePattern_);
}

string const&
RootOutput::
lastClosedFileName() const
{
  if (lastClosedFileName_.empty()) {
    throw Exception(errors::LogicError, "RootOutput::currentFileName(): ")
        << "called before meaningful.\n";
  }
  return lastClosedFileName_;
}

void
RootOutput::
beginJob()
{
  rpm_.invoke(&ResultsProducer::doBeginJob);
}

void
RootOutput::
endJob()
{
  rpm_.invoke(&ResultsProducer::doEndJob);
}

void
RootOutput::event(EventPrincipal& ep)
{
  rpm_.for_each_RPWorker([&ep](RPWorker& w) {
    w.rp().doEvent(ep);
  });
}

void
RootOutput::beginSubRun(SubRunPrincipal& srp)
{
  rpm_.for_each_RPWorker([&srp](RPWorker& w) {
    w.rp().doBeginSubRun(srp);
  });
}

void
RootOutput::endSubRun(SubRunPrincipal& srp)
{
  rpm_.for_each_RPWorker([&srp](RPWorker& w) {
    w.rp().doEndSubRun(srp);
  });
}

void
RootOutput::beginRun(RunPrincipal& rp)
{
  rpm_.for_each_RPWorker([&rp](RPWorker& w) {
    w.rp().doBeginRun(rp);
  });
}

void
RootOutput::
endRun(RunPrincipal& rp)
{
  rpm_.for_each_RPWorker([&rp](RPWorker& w) {
    w.rp().doEndRun(rp);
  });
}

} // namespace art

DEFINE_ART_MODULE(art::RootOutput)

// vim: set sw=2:
