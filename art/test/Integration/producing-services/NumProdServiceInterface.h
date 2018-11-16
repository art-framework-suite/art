#include "art/Framework/Core/ProducingService.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace arttest {
  class NumProdServiceInterface : public art::ProducingService {};
}

DECLARE_ART_SERVICE_INTERFACE(arttest::NumProdServiceInterface, LEGACY)
