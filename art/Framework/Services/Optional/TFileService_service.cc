// ======================================================================
//
// TFileService
//
// ======================================================================

#include "art/Framework/Services/Optional/TFileService.h"

#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/parent_path.h"
#include "art/Utilities/unique_filename.h"
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
  , fstats_{config.get_PSet().get<std::string>("service_type"), ServiceHandle<TriggerNamesService const>{}->getProcessName()}
  , filePattern_{config().fileName()}
  , uniqueFilename_{unique_filename((config().tmpDir() == default_tmpDir ?
                                     parent_path(filePattern_) :
                                     config().tmpDir()) + "/TFileService")}
{
  assert(file_ == nullptr && "TFile pointer should always be zero here!");
  file_ = new TFile{uniqueFilename_.c_str(), "RECREATE"};
  fstats_.recordFileOpen();
  // Activities to monitor in order to set the proper directory.
  r.sPreModuleRespondToOpenInputFile.watch   (this, &TFileService::setDirectoryName);
  r.sPreModuleRespondToCloseInputFile.watch  (this, &TFileService::setDirectoryName);
  r.sPreModuleRespondToOpenOutputFiles.watch (this, &TFileService::setDirectoryName);
  r.sPreModuleRespondToCloseOutputFiles.watch(this, &TFileService::setDirectoryName);
  r.sPreModuleConstruction.watch      (this, &TFileService::setDirectoryName);
  r.sPreModule.watch                  (this, &TFileService::setDirectoryName);
  r.sPreModuleBeginJob.watch          (this, &TFileService::setDirectoryName);
  r.sPreModuleEndJob.watch            (this, &TFileService::setDirectoryName);
  r.sPreModuleBeginRun.watch          (this, &TFileService::setDirectoryName);
  r.sPreModuleEndRun.watch            (this, &TFileService::setDirectoryName);
  r.sPreModuleBeginSubRun.watch       (this, &TFileService::setDirectoryName);
  r.sPreModuleEndSubRun.watch         (this, &TFileService::setDirectoryName);
  // Activities to monitor to keep track of events, subruns and runs seen.
  r.sPostProcessEvent.watch([this](Event  const& e ){ fstats_.recordEvent (e .id()); });
  r.sPostEndSubRun.watch   ([this](SubRun const& sr){ fstats_.recordSubRun(sr.id()); });
  r.sPostEndRun.watch      ([this](Run    const& r ){ fstats_.recordRun   (r .id()); });
}

// ----------------------------------------------------------------------
TFileService::~TFileService()
{
  file_->Write();
  if (closeFileFast_) {
    gROOT->GetListOfFiles()->Remove(file_);
  }
  file_->Close();
  delete file_;
  fstats_.recordFileClose();
  PostCloseFileRenamer{fstats_}.maybeRenameFile(uniqueFilename_, filePattern_);
}

// ----------------------------------------------------------------------

void
TFileService::setDirectoryName(ModuleDescription const& desc)
{
  dir_ = desc.moduleLabel();
  descr_ = dir_ ;
  descr_ += " (";
  descr_ += desc.moduleName();
  descr_ += ") folder";
}

// ======================================================================

DEFINE_ART_SERVICE(TFileService)

// ======================================================================
