#include "art/Framework/Services/Basic/Tracer.h"
#include "art/Framework/Services/Basic/InitRootHandlers.h"
#include "art/Framework/Services/Basic/UnixSignalService.h"

#include "art/Framework/Services/Basic/Timing.h"
#include "art/Framework/Services/Basic/Memory.h"
#include "art/Framework/Services/Basic/LoadAllDictionaries.h"
#include "art/Framework/Services/Basic/EnableFloatingPointExceptions.h"
#include "art/Framework/Services/Registry/ServiceMaker.h"

using edm::service::Tracer;
using edm::service::Timing;
using edm::service::SimpleMemoryCheck;
using edm::service::LoadAllDictionaries;
using edm::service::EnableFloatingPointExceptions;
using edm::service::InitRootHandlers;
using edm::service::UnixSignalService;

DEFINE_FWK_SERVICE(Tracer);
DEFINE_FWK_SERVICE(Timing);

#if defined(__linux__)
DEFINE_FWK_SERVICE(SimpleMemoryCheck);
typedef edm::serviceregistry::AllArgsMaker<edm::RootHandlers,InitRootHandlers> RootHandlersMaker;
DEFINE_FWK_SERVICE_MAKER(InitRootHandlers, RootHandlersMaker);
DEFINE_FWK_SERVICE(UnixSignalService);
DEFINE_FWK_SERVICE_MAKER(EnableFloatingPointExceptions,edm::serviceregistry::AllArgsMaker<EnableFloatingPointExceptions>);
#endif

DEFINE_FWK_SERVICE_MAKER(LoadAllDictionaries,edm::serviceregistry::ParameterSetMaker<LoadAllDictionaries>);
