// ======================================================================
//
// TFileService
//
// ======================================================================

#include "art/Framework/Services/Optional/TFileService.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/unique_filename.h"
#include "fhiclcpp/ParameterSet.h"

#include "TFile.h"
#include "TROOT.h"

using namespace std;
using art::TFileService;
using fhicl::ParameterSet;

// ----------------------------------------------------------------------

TFileService::TFileService(ParameterSet const & cfg,
                           ActivityRegistry & r)
  :
  TFileDirectory("", "", nullptr, ""),
  closeFileFast_(cfg.get<bool>("closeFileFast", false)),
  fileRenamer_(cfg.get<string>("fileName"),
               cfg.get<std::string>("service_type"),
               ServiceHandle<TriggerNamesService>()->getProcessName())
{
  assert(file_ == nullptr && "TFile pointer should always be zero here!");
  file_ = new TFile(unique_filename(fileRenamer_.parentPath() + "/TFileService").c_str(),
                    "RECREATE");
  // Activities to monitor in order to set the proper directory.
  r.sPreModuleConstruction.watch(this, & TFileService::setDirectoryName);
  r.sPreModule.watch            (this, & TFileService::setDirectoryName);
  r.sPreModuleBeginJob.watch    (this, & TFileService::setDirectoryName);
  r.sPreModuleEndJob.watch      (this, & TFileService::setDirectoryName);
  r.sPreModuleBeginRun.watch    (this, & TFileService::setDirectoryName);
  r.sPreModuleEndRun.watch      (this, & TFileService::setDirectoryName);
  r.sPreModuleBeginSubRun.watch (this, & TFileService::setDirectoryName);
  r.sPreModuleEndSubRun.watch   (this, & TFileService::setDirectoryName);
  // Activities to monitor to keep track of events, subruns and runs seen.
  r.sPostProcessEvent.watch     ([this](Event const & e) -> void
                                 { fileRenamer_.recordEvent(e.id()); });
  r.sPostEndSubRun.watch        ([this](SubRun const & sr) -> void
                                 { fileRenamer_.recordSubRun(sr.id()); });
  r.sPostEndRun.watch           ([this](Run const & r) -> void
                                 { fileRenamer_.recordRun(r.id()); });
}

// ----------------------------------------------------------------------

TFileService::~TFileService()
{
  file_->Write();
  if( closeFileFast_ )
    gROOT->GetListOfFiles()->Remove(file_);
  file_->Close();
  delete file_;
  fileRenamer_.applySubstitutions();
  fileRenamer_.maybeRenameFile(fullPath());
}

// ----------------------------------------------------------------------

void
  TFileService::setDirectoryName( ModuleDescription const & desc )
{
  dir_ = desc.moduleLabel();
  descr_ = (dir_ + " (" + desc.moduleName() + ") folder").c_str();
}

// ======================================================================

DEFINE_ART_SERVICE(TFileService)

// ======================================================================
