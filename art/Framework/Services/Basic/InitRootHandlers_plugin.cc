#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Framework/Services/Basic/InitRootHandlers.h"

using edm::service::InitRootHandlers;

typedef edm::serviceregistry::AllArgsMaker<edm::RootHandlers,InitRootHandlers> RootHandlersMaker;
DEFINE_FWK_SERVICE_MAKER(InitRootHandlers, RootHandlersMaker);
//DEFINE_FWK_SERVICE(InitRootHandlers);
