// ======================================================================
//
// TFileService
//
// ======================================================================

#include "art/Framework/Services/Optional/TFileService.h"

#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/IO/detail/logFileAction.h"
#include "art/Framework/IO/detail/validateFileNamePattern.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/parent_path.h"
#include "art/Utilities/unique_filename.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/assert_only_one_thread.h"
#include "fhiclcpp/ParameterSet.h"

#include "TFile.h"
#include "TROOT.h"

using namespace std;
using art::TFileService;
using fhicl::ParameterSet;

// ----------------------------------------------------------------------

TFileService::TFileService(ServiceTable<Config> const& config,
                           ActivityRegistry& r)
  : TFileDirectory{"", "", nullptr, ""}
  , closeFileFast_{config().closeFileFast()}
  , fstats_{config.get_PSet().get<std::string>("service_type"),
            ServiceHandle<TriggerNamesService const>{}->getProcessName()}
  , filePattern_{config().fileName()}
  , tmpDir_{config().tmpDir()}
{
  ClosingCriteria::Config fpConfig;
  requireCallback_ = config().fileProperties(fpConfig);
  if (requireCallback_) {
    detail::validateFileNamePattern(requireCallback_, filePattern_);
    fileSwitchCriteria_ = ClosingCriteria{fpConfig};
  }
  openFile_();

  // Activities to monitor in order to set the proper directory.
  r.sPostOpenFile.watch(
    [this](std::string const& fileName) { fstats_.recordInputFile(fileName); });

  r.sPreModuleBeginJob.watch(this, &TFileService::setDirectoryName_);
  r.sPreModuleEndJob.watch(this, &TFileService::setDirectoryName_);

  r.sPreModuleConstruction.watch(this, &TFileService::setDirectoryName_);
  r.sPreModuleRespondToOpenInputFile.watch(this,
                                           &TFileService::setDirectoryName_);
  r.sPreModuleRespondToCloseInputFile.watch(this,
                                            &TFileService::setDirectoryName_);
  r.sPreModuleRespondToOpenOutputFiles.watch(this,
                                             &TFileService::setDirectoryName_);
  r.sPreModuleRespondToCloseOutputFiles.watch(this,
                                              &TFileService::setDirectoryName_);

  r.sPreModuleBeginRun.watch(this, &TFileService::setDirectoryName_);
  r.sPreModuleEndRun.watch(this, &TFileService::setDirectoryName_);

  r.sPreModuleBeginSubRun.watch(this, &TFileService::setDirectoryName_);
  r.sPreModuleEndSubRun.watch(this, &TFileService::setDirectoryName_);

  r.sPreModule.watch(this, &TFileService::setDirectoryName_);

  // Activities to monitor to keep track of events, subruns and runs seen.
  r.sPostProcessEvent.watch([this](Event const& e) {
    currentGranularity_ = Granularity::Event;
    fp_.update_event();
    fstats_.recordEvent(e.id());
    if (requestsToCloseFile_()) {
      maybeSwitchFiles_();
    }
  });
  r.sPostEndSubRun.watch([this](SubRun const& sr) {
    currentGranularity_ = Granularity::SubRun;
    fp_.update_subRun(status_);
    fstats_.recordSubRun(sr.id());
    if (requestsToCloseFile_()) {
      maybeSwitchFiles_();
    }
  });
  r.sPostEndRun.watch([this](Run const& r) {
    currentGranularity_ = Granularity::Run;
    fp_.update_run(status_);
    fstats_.recordRun(r.id());
    if (requestsToCloseFile_()) {
      maybeSwitchFiles_();
    }
  });
  r.sPostCloseFile.watch([this] {
    currentGranularity_ = Granularity::InputFile;
    fp_.update_inputFile();
    if (requestsToCloseFile_()) {
      maybeSwitchFiles_();
    }
  });
}

TFileService::~TFileService()
{
  closeFile_();
}

void
TFileService::registerFileSwitchCallback(Callback_t cb)
{
  registerCallback(cb);
}

void
TFileService::setDirectoryName_(ModuleDescription const& desc)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  dir_ = desc.moduleLabel();
  descr_ = dir_;
  descr_ += " (";
  descr_ += desc.moduleName();
  descr_ += ") folder";
}

void
TFileService::openFile_()
{
  uniqueFilename_ = unique_filename(
    (tmpDir_ == default_tmpDir ? parent_path(filePattern_) : tmpDir_) +
    "/TFileService");
  assert(file_ == nullptr && "TFile pointer should always be zero here!");
  beginTime_ = std::chrono::steady_clock::now();
  file_ = new TFile{uniqueFilename_.c_str(), "RECREATE"};
  status_ = OutputFileStatus::Open;
  fstats_.recordFileOpen();
}

void
TFileService::closeFile_()
{
  file_->Write();
  if (closeFileFast_) {
    gROOT->GetListOfFiles()->Remove(file_);
  }
  file_->Close();
  delete file_;
  file_ = nullptr;
  status_ = OutputFileStatus::Closed;
  fstats_.recordFileClose();
  lastClosedFile_ = PostCloseFileRenamer{fstats_}.maybeRenameFile(
    uniqueFilename_, filePattern_);
}

void
TFileService::maybeSwitchFiles_()
{
  // FIXME: Should maybe include the granularity check in
  // requestsToCloseFile_().
  if (fileSwitchCriteria_.granularity() > currentGranularity_)
    return;

  status_ = OutputFileStatus::Switching;
  closeFile_();
  detail::logFileAction("Closed TFileService file ", lastClosedFile_);
  detail::logFileAction("Switching to new TFileService file with pattern ",
                        filePattern_);
  fp_ = FileProperties{};
  openFile_();
  invokeCallbacks();
}

bool
TFileService::requestsToCloseFile_()
{
  using namespace std::chrono;
  unsigned int constexpr oneK{1024u};
  fp_.updateSize(file_->GetSize() / oneK);
  fp_.updateAge(duration_cast<seconds>(steady_clock::now() - beginTime_));
  return fileSwitchCriteria_.should_close(fp_);
}

DEFINE_ART_SERVICE(TFileService)
