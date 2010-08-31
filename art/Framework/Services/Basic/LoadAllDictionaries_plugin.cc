#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Framework/Services/Basic/LoadAllDictionaries.h"

using edm::service::LoadAllDictionaries;

DEFINE_FWK_SERVICE_MAKER(LoadAllDictionaries,edm::serviceregistry::ParameterSetMaker<LoadAllDictionaries>);

