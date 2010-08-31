#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Framework/Services/Basic/LockService.h"

using edm::rootfix::LockService;

typedef edm::serviceregistry::AllArgsMaker<LockService> LockServiceMaker;

DEFINE_FWK_SERVICE_MAKER(LockService, LockServiceMaker);
