//--------------------------------------------------------------------
//
// Produces an BigProductsMaker instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"

namespace fhicl {
  class ParameterSet;
}

namespace arttest {

  class BigProductsMaker : public art::EDProducer {
  public:
    using coll_t = std::vector<double>;

    explicit BigProductsMaker(fhicl::ParameterSet const&)
    {
      produces<coll_t>();
    }

    void
    produce(art::Event& e) override
    {
      e.put(std::make_unique<coll_t>(10000, 34.));
    }
  };
}

DEFINE_ART_MODULE(arttest::BigProductsMaker)
