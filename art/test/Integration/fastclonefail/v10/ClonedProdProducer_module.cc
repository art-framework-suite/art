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
    explicit ClonedProdProducer(ParameterSet const& /*pset*/)
    {
      produces<ClonedProd>();
    }

    void produce(Event&) override;
  };

  void
  ClonedProdProducer::produce(art::Event& e)
  {
    unique_ptr<ClonedProd> prod(new ClonedProd);
    e.put(move(prod));
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ClonedProdProducer)
