#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <string>
#include <vector>

//#include "cetlib/map_vector.h"

namespace arttest {
   class UnputtingProducer;
}

//--------------------------------------------------------------------
//
// UnputtingProducer registers products, but does not put them into
// the Event.
//
class arttest::UnputtingProducer : public art::EDProducer {
public:
  explicit UnputtingProducer(fhicl::ParameterSet const&);
  virtual ~UnputtingProducer() = default;
  virtual void produce(art::Event& e) override;
};


arttest::UnputtingProducer::UnputtingProducer(fhicl::ParameterSet const& ) :
  art::EDProducer()
{
  produces<std::string>();
}

void
arttest::UnputtingProducer::produce(art::Event&) 
{
  // We don't put anything.
}

DEFINE_ART_MODULE(arttest::UnputtingProducer)
