//#include "FWCore/Services/src/SiteLocalConfigService.h"
#include "art/Framework/Services/Basic/Tracer.h"
#include "art/Framework/Services/Basic/InitRootHandlers.h"
#include "art/Framework/Services/Basic/UnixSignalService.h"

//#include "FWCore/Services/src/JobReportService.h"
#include "art/Framework/Services/Basic/Timing.h"
#include "art/Framework/Services/Basic/Memory.h"
#include "art/Framework/Services/Basic/LoadAllDictionaries.h"
#include "art/Framework/Services/Basic/EnableFloatingPointExceptions.h"
#include "art/Framework/Services/Basic/LockService.h"
#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Framework/Services/Basic/PrintLoadingPlugins.h"
#include "art/Framework/Services/Basic/UpdaterService.h"

//using edm::service::JobReportService;
using edm::service::Tracer;
using edm::service::Timing;
using edm::service::SimpleMemoryCheck;
using edm::service::LoadAllDictionaries;
//using edm::service::SiteLocalConfigService;
using edm::service::EnableFloatingPointExceptions;
using edm::service::InitRootHandlers;
using edm::service::UnixSignalService;
using edm::rootfix::LockService;

DEFINE_FWK_SERVICE(Tracer);
DEFINE_FWK_SERVICE(Timing);
DEFINE_FWK_SERVICE(UpdaterService);


typedef edm::serviceregistry::NoArgsMaker<PrintLoadingPlugins> PrintLoadingPluginsMaker;
DEFINE_FWK_SERVICE_MAKER(PrintLoadingPlugins, PrintLoadingPluginsMaker);
//typedef edm::serviceregistry::AllArgsMaker<edm::SiteLocalConfig,SiteLocalConfigService> SiteLocalConfigMaker;
//DEFINE_FWK_SERVICE_MAKER(SiteLocalConfigService,SiteLocalConfigMaker);

#if defined(__linux__)
DEFINE_FWK_SERVICE(SimpleMemoryCheck);
typedef edm::serviceregistry::AllArgsMaker<edm::RootHandlers,InitRootHandlers> RootHandlersMaker;
DEFINE_FWK_SERVICE_MAKER(InitRootHandlers, RootHandlersMaker);
DEFINE_FWK_SERVICE(UnixSignalService);
DEFINE_FWK_SERVICE_MAKER(EnableFloatingPointExceptions,edm::serviceregistry::AllArgsMaker<EnableFloatingPointExceptions>);
#endif

DEFINE_FWK_SERVICE_MAKER(LoadAllDictionaries,edm::serviceregistry::ParameterSetMaker<LoadAllDictionaries>);
//typedef edm::serviceregistry::AllArgsMaker<edm::JobReport,JobReportService> JobReportMaker;
//DEFINE_FWK_SERVICE_MAKER(JobReportService, JobReportMaker);
typedef edm::serviceregistry::AllArgsMaker<LockService> LockServiceMaker;
DEFINE_FWK_SERVICE_MAKER(LockService, LockServiceMaker);

