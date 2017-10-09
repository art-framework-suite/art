#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/TestObjects/ToyProducts.h"

namespace arttest {
  class FailingAnalyzer;
}

//--------------------------------------------------------------------
//
// throws an exception.
// Every call to FailingAnalyzer::produce throws an art::Exception
//
class arttest::FailingAnalyzer : public art::EDAnalyzer {
public:
  explicit FailingAnalyzer(fhicl::ParameterSet const&);
  void analyze(art::Event const& e) override;
  void makeTrouble(art::Event const& e);
};

arttest::FailingAnalyzer::FailingAnalyzer(fhicl::ParameterSet const& pset)
  : art::EDAnalyzer(pset)
{}

void
arttest::FailingAnalyzer::analyze(art::Event const&)
{
  // Nothing to do.
}

void
arttest::FailingAnalyzer::makeTrouble(art::Event const&)
{
  throw art::Exception(art::errors::ProductNotFound)
    << "Intentional exception for testing purposes\n";
}

DEFINE_ART_MODULE(arttest::FailingAnalyzer)
