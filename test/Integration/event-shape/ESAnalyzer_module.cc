////////////////////////////////////////////////////////////////////////
// Class:       ESAnalyzer
// Module Type: producer
// File:        ESAnalyzer_module.cc
//
// Generated at Mon Feb  3 11:00:32 2014 by Christopher Green using artmod
// from cetpkgsupport v1_05_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace arttest {
  class ESAnalyzer;
}

class arttest::ESAnalyzer : public art::EDProducer {
public:
  explicit ESAnalyzer(fhicl::ParameterSet const & p);

  void produce(art::Event & e) override;


private:

  // Declare member data here.

};


arttest::ESAnalyzer::ESAnalyzer(fhicl::ParameterSet const & p)
// :
// Initialize member data here.
{
  // Call appropriate Produces<>() functions here.
}

void arttest::ESAnalyzer::produce(art::Event & e)
{
  // Implementation of required member function here.
}

DEFINE_ART_MODULE(arttest::ESAnalyzer)
