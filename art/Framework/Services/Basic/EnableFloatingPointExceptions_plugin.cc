#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Framework/Services/Basic/EnableFloatingPointExceptions.h"

using edm::service::EnableFloatingPointExceptions;

DEFINE_FWK_SERVICE_MAKER(EnableFloatingPointExceptions,edm::serviceregistry::AllArgsMaker<EnableFloatingPointExceptions>);
