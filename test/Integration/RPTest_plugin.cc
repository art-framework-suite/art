#include "art/Framework/Core/ResultsProducer.h"

#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class RPTest;
}

class arttest::RPTest : public art::ResultsProducer {
public:
  RPTest(fhicl::ParameterSet const &) { }
  void clear() { }
private:
  void writeResults(art::Results &) override { }
};

DEFINE_ART_RESULTS_PLUGIN(arttest::RPTest)
