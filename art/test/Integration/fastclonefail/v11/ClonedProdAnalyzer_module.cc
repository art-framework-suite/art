#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/Integration/fastclonefail/v11/ClonedProd.h"
#include "fhiclcpp/ParameterSet.h"

using namespace art;
using namespace fhicl;
using namespace std;

namespace arttest {

class ClonedProdAnalyzer : public EDAnalyzer {

public:

  explicit ClonedProdAnalyzer(ParameterSet const&);
  void analyze(Event const&) override;

};

ClonedProdAnalyzer::
ClonedProdAnalyzer(ParameterSet const& pset)
  : EDAnalyzer(pset)
{
}

void
ClonedProdAnalyzer::
analyze(Event const& e)
{
  Handle<ClonedProd> h;
  e.getByLabel("ClonedProdProducer", h);
}

} // namespace arttest

DEFINE_ART_MODULE(arttest::ClonedProdAnalyzer)

