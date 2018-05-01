#include "art/Framework/Services/Optional/TFileService.h"
// vim: set sw=2 expandtab :

#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/IO/detail/logFileAction.h"
#include "art/Framework/IO/detail/validateFileNamePattern.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/parent_path.h"
#include "art/Utilities/unique_filename.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/RecursiveMutex.h"

#include "TFile.h"
#include "TROOT.h"

#include <chrono>
#include <string>

using namespace std;
using namespace hep::concurrency;
using fhicl::ParameterSet;

namespace art {

  namespace {
    string const dev_null{"/dev/null"};
  } // unnamed namespace

  TFileService::~TFileService() { closeFile_(); }

  TFileService::TFileService(ServiceTable<Config> const& config,
                             ActivityRegistry& r)
    : TFileDirectory{"", "", nullptr, ""}
    , closeFileFast_{config().closeFileFast()}
    , fstats_{config.get_PSet().get<string>("service_type"),
              Globals::instance()->processName()}
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
    r.sPostOpenFile.watch([this](string const& fileName) {
      RecursiveMutexSentry sentry{mutex_, __func__};
      fstats_.recordInputFile(fileName);
    });
    r.sPreModuleBeginJob.watch(this, &TFileService::setDirectoryName_);
    r.sPreModuleEndJob.watch(this, &TFileService::setDirectoryName_);
    r.sPreModuleConstruction.watch(this, &TFileService::setDirectoryName_);
    r.sPreModuleRespondToOpenInputFile.watch(this,
                                             &TFileService::setDirectoryName_);
    r.sPreModuleRespondToCloseInputFile.watch(this,
                                              &TFileService::setDirectoryName_);
    r.sPreModuleRespondToOpenOutputFiles.watch(
      this, &TFileService::setDirectoryName_);
    r.sPreModuleRespondToCloseOutputFiles.watch(
      this, &TFileService::setDirectoryName_);
    r.sPreModuleBeginRun.watch(this, &TFileService::setDirectoryName_);
    r.sPreModuleEndRun.watch(this, &TFileService::setDirectoryName_);
    r.sPreModuleBeginSubRun.watch(this, &TFileService::setDirectoryName_);
    r.sPreModuleEndSubRun.watch(this, &TFileService::setDirectoryName_);
    r.sPreModule.watch(this, &TFileService::setDirectoryNameWithScheduleID_);
    // Activities to monitor to keep track of events, subruns and runs seen.
    r.sPostProcessEvent.watch([this](Event const& e, ScheduleID) {
      RecursiveMutexSentry sentry{mutex_, __func__};
      currentGranularity_ = Granularity::Event;
      fp_.update_event();
      fstats_.recordEvent(e.id());
      if (requestsToCloseFile_()) {
        maybeSwitchFiles_();
      }
    });
    r.sPostEndSubRun.watch([this](SubRun const& sr) {
      RecursiveMutexSentry sentry{mutex_, __func__};
      currentGranularity_ = Granularity::SubRun;
      fp_.update_subRun(status_);
      fstats_.recordSubRun(sr.id());
      if (requestsToCloseFile_()) {
        maybeSwitchFiles_();
      }
    });
    r.sPostEndRun.watch([this](Run const& r) {
      RecursiveMutexSentry sentry{mutex_, __func__};
      currentGranularity_ = Granularity::Run;
      fp_.update_run(status_);
      fstats_.recordRun(r.id());
      if (requestsToCloseFile_()) {
        maybeSwitchFiles_();
      }
    });
    r.sPostCloseFile.watch([this] {
      RecursiveMutexSentry sentry{mutex_, __func__};
      currentGranularity_ = Granularity::InputFile;
      fp_.update_inputFile();
      if (requestsToCloseFile_()) {
        maybeSwitchFiles_();
      }
    });
  }

  void
  TFileService::registerFileSwitchCallback(Callback_t cb)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    registerCallback(cb);
  }

  void
  TFileService::setDirectoryNameWithScheduleID_(ModuleDescription const& desc,
                                                ScheduleID)
  {
    setDirectoryName_(desc);
  }

  void
  TFileService::setDirectoryName_(ModuleDescription const& desc)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    dir_ = desc.moduleLabel();
    descr_ = dir_;
    descr_ += " (";
    descr_ += desc.moduleName();
    descr_ += ") folder";
  }

  string
  TFileService::fileNameAtOpen_()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    if (filePattern_ == dev_null) {
      return dev_null;
    }
    // Use named return value optimization.
    auto ret = unique_filename(
      ((tmpDir_ == default_tmpDir) ? parent_path(filePattern_) : tmpDir_) +
      "/TFileService");
    return ret;
  }

  string
  TFileService::fileNameAtClose_(string const& filename)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    // Use named return value optimization.
    auto ret = (filePattern_ == dev_null) ?
                 dev_null :
                 fRenamer_.maybeRenameFile(filename, filePattern_);
    return ret;
  }

  void
  TFileService::openFile_()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    uniqueFilename_ = fileNameAtOpen_();
    assert((file_ == nullptr) && "TFile pointer should always be zero here!");
    beginTime_ = chrono::steady_clock::now();
    file_ = new TFile{uniqueFilename_.c_str(), "RECREATE"};
    status_ = OutputFileStatus::Open;
    fstats_.recordFileOpen();
  }

  void
  TFileService::closeFile_()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    file_->Write();
    if (closeFileFast_) {
      gROOT->GetListOfFiles()->Remove(file_);
    }
    file_->Close();
    delete file_;
    file_ = nullptr;
    status_ = OutputFileStatus::Closed;
    fstats_.recordFileClose();
    lastClosedFile_ = fileNameAtClose_(uniqueFilename_);
  }

  void
  TFileService::maybeSwitchFiles_()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    // FIXME: Should maybe include the granularity check in
    // requestsToCloseFile_().
    if (fileSwitchCriteria_.granularity() > currentGranularity_) {
      return;
    }
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
    RecursiveMutexSentry sentry{mutex_, __func__};
    using namespace chrono;
    fp_.updateSize(file_->GetSize() / 1024U);
    fp_.updateAge(duration_cast<seconds>(steady_clock::now() - beginTime_));
    return fileSwitchCriteria_.should_close(fp_);
  }

} // namespace art

DEFINE_ART_SERVICE(art::TFileService)
