#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/Integration/fastclonefail/v10/ClonedProd.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <vector>

using namespace art;
using namespace fhicl;
using namespace std;

namespace arttest {

  class ClonedProdProducer : public EDProducer {
  public:
    struct Config {
    };
    using Parameters = Table<Config>;
    explicit ClonedProdProducer(Parameters const&);

  private:
    void produce(Event&) override;
  };

  ClonedProdProducer::ClonedProdProducer(Parameters const& ps) : EDProducer{ps}
  {
    produces<ClonedProd>();
  }

  void
  ClonedProdProducer::produce(art::Event& e)
  {
    e.put(std::make_unique<ClonedProd>());
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ClonedProdProducer)
