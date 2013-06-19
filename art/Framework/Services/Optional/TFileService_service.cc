// ======================================================================
//
// TFileService
//
// ======================================================================

#include "art/Framework/Services/Optional/TFileService.h"

#include "TFile.h"
#include "TROOT.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSet.h"

using namespace std;
using art::TFileService;
using fhicl::ParameterSet;

// ----------------------------------------------------------------------

TFileService::TFileService( ParameterSet const & cfg
                          , ActivityRegistry & r
                          )
: TFileDirectory   ( ""
                   , ""
                   , new TFile( cfg.get<std::string>("fileName").c_str()
                              , "RECREATE"
                              )
                   , ""
                   )
, file_            ( TFileDirectory::file_ )
, closeFileFast_   ( cfg.get<bool>("closeFileFast", false) )
{
  // activities to monitor in order to set the proper directory
  r.sPreModuleConstruction.watch(this, & TFileService::setDirectoryName);
  r.sPreModule.watch            (this, & TFileService::setDirectoryName);
  r.sPreModuleBeginJob.watch    (this, & TFileService::setDirectoryName);
  r.sPreModuleEndJob.watch      (this, & TFileService::setDirectoryName);
  r.sPreModuleBeginRun.watch    (this, & TFileService::setDirectoryName);
  r.sPreModuleEndRun.watch      (this, & TFileService::setDirectoryName);
  r.sPreModuleBeginSubRun.watch (this, & TFileService::setDirectoryName);
  r.sPreModuleEndSubRun.watch   (this, & TFileService::setDirectoryName);
}

// ----------------------------------------------------------------------

TFileService::~TFileService()
{
  file_->Write();
  if( closeFileFast_ )
    gROOT->GetListOfFiles()->Remove(file_);
  file_->Close();
  delete file_;
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
