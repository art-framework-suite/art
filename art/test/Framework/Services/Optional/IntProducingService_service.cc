#include "art/Framework/Principal/Run.h"
#include "art/Framework/Core/ProducingService.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "art/test/TestObjects/ToyProducts.h"

namespace {
  class IntProducingService : public art::ProducingService {
  public:
    struct Config {
    };
    using Parameters = art::ServiceTable<Config>;
    IntProducingService(Parameters const&);

  private:
    void postReadRun(art::Run&) override;
  };

  IntProducingService::IntProducingService(Parameters const&)
  {
    produces<arttest::IntProduct, art::InRun>();
  }

  void
  IntProducingService::postReadRun(art::Run& r)
  {
    r.put(std::make_unique<arttest::IntProduct>(14));
  }
}

DEFINE_ART_PRODUCING_SERVICE(IntProducingService)
