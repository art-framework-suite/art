#include "art/Framework/Services/Basic/TFileService.h"

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/Service.h"
#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;

#include "TFile.h"
#include "TROOT.h"

using namespace std;
using edm::TFileService;


namespace edm {

  TFileService::TFileService(const ParameterSet & cfg, ActivityRegistry & r) :
    TFileDirectory("",
                   "",
                   new TFile(cfg.getString("fileName").c_str() ,
                             "RECREATE"),
                   ""),
    file_(TFileDirectory::file_),
    fileName_(cfg.getString("fileName")),
    fileNameRecorded_(false),
    closeFileFast_(cfg.getBool("closeFileFast", false))
  {
    // activities to monitor in order to set the proper directory
    r.watchPreModuleConstruction(this, & TFileService::setDirectoryName);
    r.watchPreModule(this, & TFileService::setDirectoryName);
    r.watchPreModuleBeginJob(this, & TFileService::setDirectoryName);
    r.watchPreModuleEndJob(this, & TFileService::setDirectoryName);
    r.watchPreModuleBeginRun(this, & TFileService::setDirectoryName);
    r.watchPreModuleEndRun(this, & TFileService::setDirectoryName);
    r.watchPreModuleBeginSubRun(this, & TFileService::setDirectoryName);
    r.watchPreModuleEndSubRun(this, & TFileService::setDirectoryName);
  }

  TFileService::~TFileService() {
    file_->Write();
    if(closeFileFast_) gROOT->GetListOfFiles()->Remove(file_);
    file_->Close();
    delete file_;
  }

  void TFileService::setDirectoryName(const ModuleDescription & desc) {
    dir_ = desc.moduleLabel_;
    descr_ = (dir_ + " (" + desc.moduleName_ + ") folder").c_str();
  }

}  // namespace edm


// ======================================================================


DEFINE_FWK_SERVICE(TFileService);
