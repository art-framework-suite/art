#include "art/Framework/Services/Basic/Tracer.h"
#include "art/Framework/Services/Basic/InitRootHandlers.h"
#include "art/Framework/Services/Basic/UnixSignalService.h"

#include "art/Framework/Services/Basic/Timing.h"
#include "art/Framework/Services/Basic/Memory.h"
#include "art/Framework/Services/Basic/LoadAllDictionaries.h"
#include "art/Framework/Services/Basic/EnableFloatingPointExceptions.h"
#include "art/Framework/Services/Registry/ServiceMaker.h"

using art::service::Tracer;
using art::service::Timing;
using art::service::SimpleMemoryCheck;
using art::service::LoadAllDictionaries;
using art::service::EnableFloatingPointExceptions;
using art::service::InitRootHandlers;
using art::service::UnixSignalService;

DEFINE_FWK_SERVICE(Tracer);
DEFINE_FWK_SERVICE(Timing);

#if defined(__linux__)
DEFINE_FWK_SERVICE(SimpleMemoryCheck);
typedef art::serviceregistry::AllArgsMaker<art::RootHandlers,InitRootHandlers> RootHandlersMaker;
DEFINE_FWK_SERVICE_MAKER(InitRootHandlers, RootHandlersMaker);
DEFINE_FWK_SERVICE(UnixSignalService);
DEFINE_FWK_SERVICE_MAKER(EnableFloatingPointExceptions,art::serviceregistry::AllArgsMaker<EnableFloatingPointExceptions>);
#endif

DEFINE_FWK_SERVICE_MAKER(LoadAllDictionaries,art::serviceregistry::ParameterSetMaker<LoadAllDictionaries>);
