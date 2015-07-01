#include "art/Framework/IO/Root/RootOutput.h"
// vim: set sw=2:

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/IO/Root/DropMetaData.h"
#include "art/Framework/IO/Root/RootOutputFile.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/unique_filename.h"
#include "art/Utilities/parent_path.h"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include <iomanip>
#include <sstream>

using std::string;

namespace art {

RootOutput::
~RootOutput()
{
}

RootOutput::
RootOutput(Parameters const & config)
  : OutputModule(config)
  , catalog_(config().catalog())
  , dropAllEvents_(false)
  , dropAllSubRuns_(config().dropAllSubRuns())
  , moduleLabel_(config.get_PSet().get<string>("module_label"))
  , inputFileCount_(0)
  , rootOutputFile_()
  , fstats_(moduleLabel_, processName())
    // For the next data member, qualifying 'fileName' is a necessity
    // because the full configuration for RootOutput looks like:
    //
    //   struct Config : OutputModuleConfig, RootOutputConfig {};
    //
    // Both OutputModuleConfig and RootOutputConfig include 'fileName'
    // members, creating a lookup ambiguity.
  , filePattern_(config().RootOutput::Config::fileName() )
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
postSelectProducts(FileBlock const& fb)
{
  if (isFileOpen()) {
    rootOutputFile_->selectProducts(fb);
  }
}

void
RootOutput::
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
RootOutput::
respondToCloseInputFile(FileBlock const& fb)
{
  if (rootOutputFile_) {
    rootOutputFile_->respondToCloseInputFile(fb);
  }
}

void
RootOutput::
write(EventPrincipal const& e)
{
  if (dropAllEvents_) {
    return;
  }
  if (hasNewlyDroppedBranch()[InEvent]) {
    e.addToProcessHistory();
  }
  rootOutputFile_->writeOne(e);
  fstats_.recordEvent(e.id());
}

void
RootOutput::
writeSubRun(SubRunPrincipal const& sr)
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
RootOutput::
writeRun(RunPrincipal const& r)
{
  if (hasNewlyDroppedBranch()[InRun]) {
    r.addToProcessHistory();
  }
  rootOutputFile_->writeRun(r);
  fstats_.recordRun(r.id());
}

void
RootOutput::
startEndFile()
{
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
  rootOutputFile_->writeParameterSetRegistry();
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
writeBranchIDListRegistry()
{
  rootOutputFile_->writeBranchIDListRegistry();
}

void
RootOutput::
doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const& md,
                           FileCatalogMetadata::collection_type const& ssmd)
{
  rootOutputFile_->writeFileCatalogMetadata(fstats_, md, ssmd);
}

void
RootOutput::
writeProductDependencies()
{
  rootOutputFile_->writeProductDependencies();
}

void
RootOutput::
finishEndFile()
{
  rootOutputFile_->finishEndFile();
  fstats_.recordFileClose();
  lastClosedFileName_ = PostCloseFileRenamer(fstats_).maybeRenameFile(
                          rootOutputFile_->currentFileName(), filePattern_);
  rootOutputFile_.reset();
}

bool
RootOutput::
isFileOpen() const
{
  return rootOutputFile_.get() != 0;
}

bool
RootOutput::
shouldWeCloseFile() const
{
  return rootOutputFile_->shouldWeCloseFile();
}

void
RootOutput::
doOpenFile()
{
  if (inputFileCount_ == 0) {
    throw art::Exception(art::errors::LogicError)
        << "Attempt to open output file before input file. "
        << "Please report this to the core framework developers.\n";
  }
  rootOutputFile_.reset(
    new RootOutputFile(this, unique_filename(tmpDir_ + "/RootOutput"),
                       maxFileSize_, compressionLevel_,
                       saveMemoryObjectThreshold_, treeMaxVirtualSize_,
                       splitLevel_, basketSize_, dropMetaData_,
                       dropMetaDataForDroppedData_, fastCloning_));
  fstats_.recordFileOpen();
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

} // namespace art


DEFINE_ART_MODULE(art::RootOutput)

