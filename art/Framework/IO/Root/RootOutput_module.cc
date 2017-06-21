#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Core/ResultsProducer.h"
#include "art/Framework/Core/RPManager.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/RootOutputFile.h"
#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"
#include "art/Framework/IO/Root/detail/rootOutputConfigurationTools.h"
#include "art/Framework/IO/detail/logFileAction.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/ConfigurationTable.h"
#include "art/Utilities/parent_path.h"
#include "art/Utilities/unique_filename.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Table.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

using std::string;

namespace art {
  class RootOutput;
  class RootOutputFile;
}

class art::RootOutput final : public OutputModule {
public:

  static constexpr char const* default_tmpDir {"<parent-path-of-filename>"};

  struct Config {

    using Name = fhicl::Name;
    using Comment = fhicl::Comment;
    template <typename T> using Atom = fhicl::Atom<T>;
    template <typename T> using OptionalAtom = fhicl::OptionalAtom<T>;

    fhicl::TableFragment<art::OutputModule::Config> omConfig;
    Atom<std::string> catalog { Name("catalog"), "" };
    OptionalAtom<bool> dropAllEvents { Name("dropAllEvents") };
    Atom<bool> dropAllSubRuns { Name("dropAllSubRuns"), false };
    OptionalAtom<bool> fastCloning { Name("fastCloning") };
    Atom<std::string> tmpDir { Name("tmpDir"), default_tmpDir };
    Atom<int> compressionLevel { Name("compressionLevel"), 7 };
    Atom<int64_t> saveMemoryObjectThreshold { Name("saveMemoryObjectThreshold"), -1l };
    Atom<int64_t> treeMaxVirtualSize { Name("treeMaxVirtualSize"), -1 };
    Atom<int> splitLevel { Name("splitLevel"), 99 };
    Atom<int> basketSize { Name("basketSize"), 16384 };
    Atom<bool> dropMetaDataForDroppedData { Name("dropMetaDataForDroppedData"), false };
    Atom<std::string> dropMetaData { Name("dropMetaData"), "NONE" };
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
      adjustFilename->set_value_type(fhicl::value_type::REQUIRED);
    }

    struct KeysToIgnore {
      std::set<std::string> operator()() const
      {
        std::set<std::string> keys {art::OutputModule::Config::KeysToIgnore::get()};
        keys.insert("results");
        return keys;
      }
    };

  };

  using Parameters = art::WrappedTable<Config,Config::KeysToIgnore>;

  explicit RootOutput(Parameters const&);

  void postSelectProducts(FileBlock const&) override;

  void beginJob() override;
  void endJob() override;

  void event(EventPrincipal const&) override;

  void beginSubRun(SubRunPrincipal const&) override;
  void endSubRun(SubRunPrincipal const&) override;

  void beginRun(RunPrincipal const&) override;
  void endRun(RunPrincipal const&) override;

private:

  std::string const& lastClosedFileName() const override;
  void openFile(FileBlock const&) override;
  void respondToOpenInputFile(FileBlock const&) override;
  void readResults(ResultsPrincipal const& resp) override;
  void respondToCloseInputFile(FileBlock const&) override;
  void incrementInputFileNumber() override;
  Granularity fileGranularity() const override;
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
                             FileCatalogMetadata::collection_type const& ssmd)
                             override;
  void writeProductDependencies() override;
  void finishEndFile() override;
  void doRegisterProducts(MasterProductRegistry& mpr,
                          ModuleDescription const& md) override;

private:

  std::string const catalog_;
  bool dropAllEvents_ {false};
  bool dropAllSubRuns_;
  std::string const moduleLabel_;
  int inputFileCount_ {0};
  std::unique_ptr<RootOutputFile> rootOutputFile_ {nullptr};
  FileStatsCollector fstats_;
  std::string const filePattern_;
  std::string tmpDir_;
  std::string lastClosedFileName_ {};

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
  bool fastCloningEnabled_ {true};

  // Set false only for cases where we are guaranteed never to need
  // historical ParameterSet information in the downstream file
  // (e.g. mixing).
  bool writeParameterSets_;
  ClosingCriteria fileProperties_;

  // ResultsProducer management.
  RPManager rpm_;
};

art::RootOutput::RootOutput(Parameters const& config)
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
  , fileProperties_{
    (detail::validateFileNamePattern(config.get_PSet().has_key(config().fileProperties.name()), filePattern_), // comma operator!
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
  fastCloningEnabled_ = detail::shouldFastClone(fastCloningSet, fastCloningEnabled_, wantAllEvents(), fileProperties_);

  if (!writeParameterSets_) {
    mf::LogWarning("PROVENANCE")
      << "Output module " << moduleLabel_ << " has parameter writeParameterSets set to false.\n"
      << "Parameter set provenance will not be available in subsequent jobs.\n"
      << "Check your experiment's policy on this issue to avoid future problems\n"
      << "with analysis reproducibility.\n";
  }
}

void
art::RootOutput::openFile(FileBlock const& fb)
{
  // Note: The file block here refers to the currently open input
  //       file, so we can find out about the available products by
  //       looping over the branches of the input file data trees.
  if (!isFileOpen()) {
    doOpenFile();
    respondToOpenInputFile(fb);
  }
}

void
art::RootOutput::postSelectProducts(FileBlock const&)
{
  if (isFileOpen()) {
    rootOutputFile_->selectProducts();
  }
}

void
art::RootOutput::respondToOpenInputFile(FileBlock const& fb)
{
  ++inputFileCount_;
  if (!isFileOpen()) return;

  bool fastCloneThisOne = fastCloningEnabled_ && (fb.tree() != nullptr) &&
                          ((remainingEvents() < 0) ||
                           (remainingEvents() >= fb.tree()->GetEntries()));
  if (fastCloningEnabled_ && !fastCloneThisOne) {
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated for this input file due to "
      << "empty event tree and/or event limits.";
  }
  if (fastCloneThisOne && !fb.fastClonable()) {
    mf::LogWarning("FastCloning")
      << "Fast cloning deactivated for this input file due to "
      << "information in FileBlock.";
    fastCloneThisOne = false;
  }
  rootOutputFile_->beginInputFile(fb, fastCloneThisOne);
  fstats_.recordInputFile(fb.fileName());
}

void
art::RootOutput::readResults(ResultsPrincipal const& resp)
{
  rpm_.for_each_RPWorker([&resp](RPWorker& w) {
      Results const res {resp, w.moduleDescription()};
      w.rp().doReadResults(res);
    } );
}

void
art::RootOutput::respondToCloseInputFile(FileBlock const& fb)
{
  if (isFileOpen()) {
    rootOutputFile_->respondToCloseInputFile(fb);
  }
}

void
art::RootOutput::write(EventPrincipal& ep)
{
  if (dropAllEvents_) {
    return;
  }
  if (hasNewlyDroppedBranch()[InEvent]) {
    ep.addToProcessHistory();
  }
  rootOutputFile_->writeOne(ep);
  fstats_.recordEvent(ep.id());
}

void
art::RootOutput::setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
{
  rootOutputFile_->setSubRunAuxiliaryRangeSetID(rs);
}

void
art::RootOutput::writeSubRun(SubRunPrincipal& sr)
{
  if (dropAllSubRuns_) {
    return;
  }
  if (hasNewlyDroppedBranch()[InSubRun]) {
    sr.addToProcessHistory();
  }
  rootOutputFile_->writeSubRun(sr);
  fstats_.recordSubRun(sr.id());
}

void
art::RootOutput::setRunAuxiliaryRangeSetID(RangeSet const& rs)
{
  rootOutputFile_->setRunAuxiliaryRangeSetID(rs);
}

void
art::RootOutput::writeRun(RunPrincipal& r)
{
  if (hasNewlyDroppedBranch()[InRun]) {
    r.addToProcessHistory();
  }
  rootOutputFile_->writeRun(r);
  fstats_.recordRun(r.id());
}

void
art::RootOutput::startEndFile()
{
  auto resp = std::make_unique<ResultsPrincipal>(ResultsAuxiliary{},
                                                 description().processConfiguration());
  if (ProductMetaData::instance().productProduced(InResults) ||
      hasNewlyDroppedBranch()[InResults]) {
    resp->addToProcessHistory();
  }
  rpm_.for_each_RPWorker([&resp](RPWorker& w) {
      Results res {*resp, w.moduleDescription()};
      w.rp().doWriteResults(*resp, res);
    } );
  rootOutputFile_->writeResults(*resp);
}

void
art::RootOutput::writeFileFormatVersion()
{
  rootOutputFile_->writeFileFormatVersion();
}

void
art::RootOutput::writeFileIndex()
{
  rootOutputFile_->writeFileIndex();
}

void
art::RootOutput::writeEventHistory()
{
  rootOutputFile_->writeEventHistory();
}

void
art::RootOutput::writeProcessConfigurationRegistry()
{
  rootOutputFile_->writeProcessConfigurationRegistry();
}

void
art::RootOutput::writeProcessHistoryRegistry()
{
  rootOutputFile_->writeProcessHistoryRegistry();
}

void
art::RootOutput::writeParameterSetRegistry()
{
  if (writeParameterSets_) {
    rootOutputFile_->writeParameterSetRegistry();
  }
}

void
art::RootOutput::writeProductDescriptionRegistry()
{
  rootOutputFile_->writeProductDescriptionRegistry();
}

void
art::RootOutput::writeParentageRegistry()
{
  rootOutputFile_->writeParentageRegistry();
}

void
art::RootOutput::doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                           FileCatalogMetadata::collection_type const& ssmd)
{
  rootOutputFile_->writeFileCatalogMetadata(fstats_, md, ssmd);
}

void
art::RootOutput::writeProductDependencies()
{
  rootOutputFile_->writeProductDependencies();
}

void
art::RootOutput::finishEndFile()
{
  std::string const currentFileName {rootOutputFile_->currentFileName()};
  rootOutputFile_->writeTTrees();
  rootOutputFile_.reset();
  fstats_.recordFileClose();
  lastClosedFileName_ = PostCloseFileRenamer{fstats_}.maybeRenameFile(currentFileName, filePattern_);
  detail::logFileAction("Closed output file ", lastClosedFileName_);
  rpm_.invoke(&ResultsProducer::doClear);
}

void
art::RootOutput::doRegisterProducts(MasterProductRegistry& mpr,
                   ModuleDescription const& md)
{
  // Register Results products from ResultsProducers.
  rpm_.for_each_RPWorker([&mpr, &md](RPWorker& w) {
      auto const& params = w.params();
      w.setModuleDescription(ModuleDescription{params.rpPSetID,
            params.rpPluginType,
            md.moduleLabel() + '#' + params.rpLabel,
            md.processConfiguration(),
            ModuleDescription::invalidID()});
      w.rp().registerProducts(mpr, w.moduleDescription());
    });
}

void
art::RootOutput::setFileStatus(OutputFileStatus const ofs)
{
  if (isFileOpen())
    rootOutputFile_->setFileStatus(ofs);
}

bool
art::RootOutput::isFileOpen() const
{
  return rootOutputFile_.get() != nullptr;
}

void
art::RootOutput::incrementInputFileNumber()
{
  if (isFileOpen())
    rootOutputFile_->incrementInputFileNumber();
}

bool
art::RootOutput::requestsToCloseFile() const
{
  return isFileOpen() ? rootOutputFile_->requestsToCloseFile() : false;
}

art::Granularity
art::RootOutput::fileGranularity() const
{
  return fileProperties_.granularity();
}

void
art::RootOutput::doOpenFile()
{
  if (inputFileCount_ == 0) {
    throw art::Exception(art::errors::LogicError)
        << "Attempt to open output file before input file. "
        << "Please report this to the core framework developers.\n";
  }
  rootOutputFile_ = std::make_unique<RootOutputFile>(this,
                                                     unique_filename(tmpDir_ + "/RootOutput"),
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
  detail::logFileAction("Opened output file with pattern ", filePattern_);
}

string const&
art::RootOutput::lastClosedFileName() const
{
  if (lastClosedFileName_.empty()) {
    throw Exception(errors::LogicError, "RootOutput::currentFileName(): ")
        << "called before meaningful.\n";
  }
  return lastClosedFileName_;
}

void
art::RootOutput::beginJob()
{
  rpm_.invoke(&ResultsProducer::doBeginJob);
}

void
art::RootOutput::endJob()
{
  rpm_.invoke(&ResultsProducer::doEndJob);
}

void
art::RootOutput::event(EventPrincipal const& ep)
{
  rpm_.for_each_RPWorker([&ep](RPWorker& w) {
      Event const e {ep, w.moduleDescription()};
      w.rp().doEvent(e);
    });
}

void
art::RootOutput::beginSubRun(art::SubRunPrincipal const& srp)
{
  rpm_.for_each_RPWorker([&srp](RPWorker& w) {
      SubRun const sr {srp, w.moduleDescription()};
      w.rp().doBeginSubRun(sr);
    });
}

void
art::RootOutput::endSubRun(art::SubRunPrincipal const& srp)
{
  rpm_.for_each_RPWorker([&srp](RPWorker& w) {
      SubRun const sr {srp, w.moduleDescription()};
      w.rp().doEndSubRun(sr);
    });
}

void
art::RootOutput::beginRun(art::RunPrincipal const& rp)
{
  rpm_.for_each_RPWorker([&rp](RPWorker& w) {
      Run const r {rp, w.moduleDescription()};
      w.rp().doBeginRun(r);
    });
}

void
art::RootOutput::endRun(art::RunPrincipal const& rp)
{
  rpm_.for_each_RPWorker([&rp](RPWorker& w) {
      Run const r {rp, w.moduleDescription()};
      w.rp().doEndRun(r);
    });
}

DEFINE_ART_MODULE(art::RootOutput)

// vim: set sw=2:
