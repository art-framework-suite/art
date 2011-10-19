#include "art/Framework/Core/EDAnalyzer.h"
#include "test/TestObjects/ToyProducts.h"
#include "art/Framework/Core/ModuleMacros.h"

namespace arttest {
   class FailingAnalyzer;
}

//--------------------------------------------------------------------
//
// throws an exception.
// Every call to FailingAnalyzer::produce throws an art::Exception
//
class arttest::FailingAnalyzer : public art::EDAnalyzer
{
public:
  explicit FailingAnalyzer(fhicl::ParameterSet const&);
  virtual ~FailingAnalyzer();
  virtual void analyze(art::Event const& e);
  void makeTrouble(art::Event const& e);
};


arttest::FailingAnalyzer::FailingAnalyzer(fhicl::ParameterSet const&)
{

}

arttest::FailingAnalyzer::~FailingAnalyzer()
{ }



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


DEFINE_ART_MODULE(arttest::FailingAnalyzer);
