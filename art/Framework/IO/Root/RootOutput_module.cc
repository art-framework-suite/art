#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Core/ResultsProducer.h"
#include "art/Framework/Core/RPManager.h"
#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/RootOutputFile.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/ConfigTable.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/parent_path.h"
#include "art/Utilities/unique_filename.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

using std::string;

namespace art {
  class RootOutput;

  class RootOutputFile; // Forward declaration.
}

class art::RootOutput : public OutputModule {

public: // MEMBER FUNCTIONS

  static constexpr const char* default_tmpDir = "<some-tmp-dir>";

  struct Config {

    fhicl::TableFragment<art::OutputModule::Config> omConfig;
    fhicl::Atom<std::string> catalog { fhicl::Name("catalog"), "" };
    fhicl::Atom<bool> dropAllEvents  { fhicl::Name("dropAllEvents"), false };
    fhicl::Atom<bool> dropAllSubRuns { fhicl::Name("dropAllSubRuns"), false };
    fhicl::Atom<std::string> tmpDir { fhicl::Name("tmpDir"), default_tmpDir };
    fhicl::Atom<int> maxSize { fhicl::Name("maxSize"), 0x7f000000 };
    fhicl::Atom<int> compressionLevel { fhicl::Name("compressionLevel"), 7 };
    fhicl::Atom<int64_t> saveMemoryObjectThreshold { fhicl::Name("saveMemoryObjectThreshold"), -1l };
    fhicl::Atom<int64_t> treeMaxVirtualSize { fhicl::Name("treeMaxVirtualSize"), -1 };
    fhicl::Atom<int> splitLevel { fhicl::Name("splitLevel"), 99 };
    fhicl::Atom<int> basketSize { fhicl::Name("basketSize"), 16384 };
    fhicl::Atom<bool> dropMetaDataForDroppedData { fhicl::Name("dropMetaDataForDroppedData"), false };
    fhicl::Atom<std::string> dropMetaData { fhicl::Name("dropMetaData"), "" };
    fhicl::Atom<bool> writeParameterSets {fhicl::Name("writeParameterSets"), true };

    Config()
    {
      // Both RootOutput module and OutputModule use the "fileName"
      // FHiCL parameter.  However, whereas in OutputModule the
      // parameter has a default, for RootOutput the parameter should
      // not.  We therefore have to change the default flag setting
      // for 'OutputModule::Config::fileName'.

      using namespace fhicl::detail;
      ParameterBase* adjustFilename = const_cast<fhicl::Atom<std::string>*>(&omConfig().fileName);
      adjustFilename->set_value_type(fhicl::value_type::REQUIRED);
    }

    struct KeysToIgnore {
      std::set<std::string> operator()()
      {
        std::set<std::string> result { art::OutputModule::Config::KeysToIgnore::get() };
        result.insert({"fastCloning", "results"});
        return result;
      }
    };

  };

  using Parameters = art::ConfigTable<Config,Config::KeysToIgnore>;

  explicit RootOutput(Parameters const&);

  void postSelectProducts(FileBlock const&) override;

  void beginJob() override;
  void endJob() override;

  void beginSubRun(SubRunPrincipal const &) override;
  void endSubRun(SubRunPrincipal const &) override;

  void beginRun(RunPrincipal const &) override;
  void endRun(RunPrincipal const &) override;

private: // MEMBER FUNCTIONS

  std::string const& lastClosedFileName() const override;
  void openFile(FileBlock const&) override;
  void respondToOpenInputFile(FileBlock const &) override;
  void readResults(ResultsPrincipal const & resp) override;
  void respondToCloseInputFile(FileBlock const&) override;
  void respondToCloseOutputFiles(FileBlock const&) override;
  void write(EventPrincipal &) override;
  void writeSubRun(SubRunPrincipal &) override;
  void writeRun(RunPrincipal &) override;
  bool isFileOpen() const override;
  bool shouldWeCloseFile() const override;
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
  void writeBranchIDListRegistry() override;
  void
  doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                             FileCatalogMetadata::collection_type const& ssmd)
                             override;
  void writeProductDependencies() override;
  void finishEndFile() override;
  void doRegisterProducts(MasterProductRegistry & mpr,
                          ModuleDescription const & md) override;

private:

  std::string const catalog_;
  bool dropAllEvents_;
  bool dropAllSubRuns_;
  std::string const moduleLabel_;
  int inputFileCount_;
  std::unique_ptr<RootOutputFile> rootOutputFile_;
  FileStatsCollector fstats_;
  std::string const filePattern_;
  std::string tmpDir_;
  std::string lastClosedFileName_;

  // We keep this set of data members for the use
  // of RootOutputFile.
  unsigned int const maxFileSize_;
  int const compressionLevel_;
  int64_t const saveMemoryObjectThreshold_;
  int64_t const treeMaxVirtualSize_;
  int const splitLevel_;
  int const basketSize_;
  DropMetaData dropMetaData_;
  bool dropMetaDataForDroppedData_;

  // We keep this for the use of RootOutputFile
  // and we also use it during file open to
  // make some choices.
  bool fastCloning_;

  // Set false only for cases where we are guaranteed never to need
  // historical ParameterSet information in the downstream file
  // (e.g. mixing).
  bool writeParameterSets_;

  // ResultsProducer management.
  RPManager rpm_;
};

art::RootOutput::
RootOutput(Parameters const & config)
  : OutputModule{config().omConfig, config.get_PSet()}
  , catalog_(config().catalog())
  , dropAllEvents_(false)
  , dropAllSubRuns_(config().dropAllSubRuns())
  , moduleLabel_(config.get_PSet().get<string>("module_label"))
  , inputFileCount_(0)
  , rootOutputFile_()
  , fstats_(moduleLabel_, processName())
  , filePattern_(config().omConfig().fileName() )
  , tmpDir_( config().tmpDir() == default_tmpDir ?
             parent_path(filePattern_) :
             config().tmpDir() )
  , lastClosedFileName_()
  , maxFileSize_(config().maxSize())
  , compressionLevel_(config().compressionLevel())
  , saveMemoryObjectThreshold_(config().saveMemoryObjectThreshold())
  , treeMaxVirtualSize_(config().treeMaxVirtualSize())
  , splitLevel_(config().splitLevel())
  , basketSize_(config().basketSize())
  , dropMetaData_(DropMetaData::DropNone)
  , dropMetaDataForDroppedData_(config().dropMetaDataForDroppedData())
  , fastCloning_(true)
  , writeParameterSets_(config().writeParameterSets())
  , rpm_(config.get_PSet())
{
  mf::LogInfo msg("FastCloning");
  msg << "Initial fast cloning configuration ";
  if (config.get_PSet().get_if_present("fastCloning", fastCloning_)) {
    msg << "(user-set): ";
  }
  else {
    msg << "(from default): ";
  }
  msg << std::boolalpha
      << fastCloning_;
  if (fastCloning_ && !wantAllEvents()) {
    fastCloning_ = false;
    mf::LogWarning("FastCloning")
        << "Fast cloning deactivated due to presence of "
        "event selection configuration.";
  }

  bool const dropAllEventsSet = config.get_PSet().get_if_present<bool>("dropAllEvents",
                                                                                    dropAllEvents_);
  if (dropAllSubRuns_) {
    if (dropAllEventsSet && !dropAllEvents_) {
      string const errmsg =
        "\nThe following FHiCL specification is illegal\n\n"
        "   dropAllEvents  : false \n"
        "   dropAllSubRuns : true  \n\n"
        "[1] Both can be 'true', "
        "[2] both can be 'false', or "
        "[3] 'dropAllEvents : true' and 'dropAllSubRuns : false' "
        "is allowed.\n\n";
      throw art::Exception(errors::Configuration, errmsg);
    }
    dropAllEvents_ = true;
  }

  string dropMetaData( config().dropMetaData() );
  if (dropMetaData.empty()) {
    dropMetaData_ = DropMetaData::DropNone;
  }
  else if (dropMetaData == string("NONE")) {
    dropMetaData_ = DropMetaData::DropNone;
  }
  else if (dropMetaData == string("PRIOR")) {
    dropMetaData_ = DropMetaData::DropPrior;
  }
  else if (dropMetaData == string("ALL")) {
    dropMetaData_ = DropMetaData::DropAll;
  }
  else {
    throw art::Exception(errors::Configuration,
                         "Illegal dropMetaData parameter value: ")
      << dropMetaData << ".\n"
      << "Legal values are 'NONE', 'PRIOR', and 'ALL'.\n";
  }

  if (!writeParameterSets_) {
    mf::LogWarning("PROVENANCE")
      << "Output module " << moduleLabel_ << " has parameter writeParameterSets set to false.\n"
      << "Parameter set provenance will not be available in subsequent jobs.\n"
      << "Check your experiment's policy on this issue to avoid future problems\n"
      << "with analysis reproducibility.\n";
  }
}

void
art::RootOutput::
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
art::RootOutput::
postSelectProducts(FileBlock const& fb)
{
  if (isFileOpen()) {
    rootOutputFile_->selectProducts(fb);
  }
}

void
art::RootOutput::
respondToOpenInputFile(FileBlock const& fb)
{
  ++inputFileCount_;
  if (isFileOpen()) {
    bool fastCloneThisOne = fastCloning_ && (fb.tree() != 0) &&
                            ((remainingEvents() < 0) ||
                             (remainingEvents() >= fb.tree()->GetEntries()));
    if (fastCloning_ && !fastCloneThisOne) {
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
    rootOutputFile_->beginInputFile(fb, fastCloneThisOne && fastCloning_);
    fstats_.recordInputFile(fb.fileName());
  }
}

void
art::RootOutput::
readResults(ResultsPrincipal const & resp)
{
  rpm_.for_each_RPWorker([&resp](RPWorker & w) {
      Results const res(const_cast<ResultsPrincipal&>(resp), w.moduleDescription());
      w.rp().doReadResults(res);
    } );
}

void
art::RootOutput::
respondToCloseInputFile(FileBlock const& fb)
{
  if (rootOutputFile_) {
    rootOutputFile_->respondToCloseInputFile(fb);
  }
}

void
art::RootOutput::
respondToCloseOutputFiles(FileBlock const&)
{
  auto resp =
    std::make_unique<ResultsPrincipal>(ResultsAuxiliary { },
                                       description().processConfiguration());
  if (ProductMetaData::instance().productProduced(InResults) ||
      hasNewlyDroppedBranch()[InResults]) {
    resp->addToProcessHistory();
  }
  rpm_.for_each_RPWorker([&resp](RPWorker & w) {
      Results res(*resp, w.moduleDescription());
      w.rp().doWriteResults(res);
    } );
  rootOutputFile_->writeResults(*resp);
}

void
art::RootOutput::
write(EventPrincipal & ep)
{
  rpm_.for_each_RPWorker([&ep](RPWorker & w) {
      Event const e(const_cast<EventPrincipal &>(ep), w.moduleDescription());
      w.rp().doEvent(e);
    });
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
art::RootOutput::
writeSubRun(SubRunPrincipal & sr)
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
art::RootOutput::
writeRun(RunPrincipal & r)
{
  if (hasNewlyDroppedBranch()[InRun]) {
    r.addToProcessHistory();
  }
  rootOutputFile_->writeRun(r);
  fstats_.recordRun(r.id());
}

void
art::RootOutput::
startEndFile()
{
}

void
art::RootOutput::
writeFileFormatVersion()
{
  rootOutputFile_->writeFileFormatVersion();
}

void
art::RootOutput::
writeFileIndex()
{
  rootOutputFile_->writeFileIndex();
}

void
art::RootOutput::
writeEventHistory()
{
  rootOutputFile_->writeEventHistory();
}

void
art::RootOutput::
writeProcessConfigurationRegistry()
{
  rootOutputFile_->writeProcessConfigurationRegistry();
}

void
art::RootOutput::
writeProcessHistoryRegistry()
{
  rootOutputFile_->writeProcessHistoryRegistry();
}

void
art::RootOutput::
writeParameterSetRegistry()
{
  if (writeParameterSets_) {
    rootOutputFile_->writeParameterSetRegistry();
  }
}

void
art::RootOutput::
writeProductDescriptionRegistry()
{
  rootOutputFile_->writeProductDescriptionRegistry();
}

void
art::RootOutput::
writeParentageRegistry()
{
  rootOutputFile_->writeParentageRegistry();
}

void
art::RootOutput::
writeBranchIDListRegistry()
{
  rootOutputFile_->writeBranchIDListRegistry();
}

void
art::RootOutput::
doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                           FileCatalogMetadata::collection_type const& ssmd)
{
  rootOutputFile_->writeFileCatalogMetadata(fstats_, md, ssmd);
}

void
art::RootOutput::
writeProductDependencies()
{
  rootOutputFile_->writeProductDependencies();
}

void
art::RootOutput::
finishEndFile()
{
  rootOutputFile_->finishEndFile();
  fstats_.recordFileClose();
  lastClosedFileName_ = PostCloseFileRenamer(fstats_).maybeRenameFile(
                          rootOutputFile_->currentFileName(), filePattern_);
  rootOutputFile_.reset();
  rpm_.invoke(&ResultsProducer::doClear);
}

void
art::RootOutput::
doRegisterProducts(MasterProductRegistry & mpr,
                   ModuleDescription const & md)
{
  // Register Results products from ResultsProducers.
  rpm_.for_each_RPWorker([&mpr, &md](RPWorker & w) {
      auto const & params = w.params();
      w.setModuleDescription(ModuleDescription(params.rpPSetID,
                                               params.rpPluginType,
                                               md.moduleLabel() + '#' + params.rpLabel,
                                               md.processConfiguration()));
      w.rp().registerProducts(mpr, w.moduleDescription());
    });
}

bool
art::RootOutput::
isFileOpen() const
{
  return rootOutputFile_.get() != 0;
}

bool
art::RootOutput::
shouldWeCloseFile() const
{
  return rootOutputFile_->shouldWeCloseFile();
}

void
art::RootOutput::
doOpenFile()
{
  if (inputFileCount_ == 0) {
    throw art::Exception(art::errors::LogicError)
        << "Attempt to open output file before input file. "
        << "Please report this to the core framework developers.\n";
  }
  rootOutputFile_ = std::make_unique<RootOutputFile>(this, unique_filename(tmpDir_ + "/RootOutput"),
                                                     maxFileSize_, compressionLevel_,
                                                     saveMemoryObjectThreshold_, treeMaxVirtualSize_,
                                                     splitLevel_, basketSize_, dropMetaData_,
                                                     dropMetaDataForDroppedData_, fastCloning_);
  fstats_.recordFileOpen();
}

string const&
art::RootOutput::
lastClosedFileName() const
{
  if (lastClosedFileName_.empty()) {
    throw Exception(errors::LogicError, "RootOutput::currentFileName(): ")
        << "called before meaningful.\n";
  }
  return lastClosedFileName_;
}

void
art::RootOutput::
beginJob()
{
  rpm_.invoke(&ResultsProducer::doBeginJob);
}

void
art::RootOutput::
endJob()
{
  rpm_.invoke(&ResultsProducer::doEndJob);
}

void
art::RootOutput::
beginSubRun(art::SubRunPrincipal const & srp)
{
  rpm_.for_each_RPWorker([&srp](RPWorker & w) {
      SubRun const sr(const_cast<SubRunPrincipal &>(srp), w.moduleDescription());
      w.rp().doBeginSubRun(sr);
    });
}

void
art::RootOutput::
endSubRun(art::SubRunPrincipal const & srp)
{
  rpm_.for_each_RPWorker([&srp](RPWorker & w) {
      SubRun const sr(const_cast<SubRunPrincipal &>(srp), w.moduleDescription());
      w.rp().doEndSubRun(sr);
    });
}

void
art::RootOutput::
beginRun(art::RunPrincipal const & rp)
{
  rpm_.for_each_RPWorker([&rp](RPWorker & w) {
      Run const r(const_cast<RunPrincipal &>(rp), w.moduleDescription());
      w.rp().doBeginRun(r);
    });
}

void
art::RootOutput::
endRun(art::RunPrincipal const & rp)
{
  rpm_.for_each_RPWorker([&rp](RPWorker & w) {
      Run const r(const_cast<RunPrincipal &>(rp), w.moduleDescription());
      w.rp().doEndRun(r);
    });
}

DEFINE_ART_MODULE(art::RootOutput)

// vim: set sw=2:
