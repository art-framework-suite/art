////////////////////////////////////////////////////////////////////////
// Class:       PMTestAnalyzer
// Module Type: analyzer
// File:        PMTestAnalyzer_module.cc
//
// Generated at Mon Jun 17 16:44:57 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class PMTestAnalyzer;
}

class arttest::PMTestAnalyzer : public art::EDAnalyzer {
public:
  explicit PMTestAnalyzer(fhicl::ParameterSet const& p);

private:
  void analyze(art::Event const& e) override;
};

arttest::PMTestAnalyzer::PMTestAnalyzer(fhicl::ParameterSet const& p)
  : art::EDAnalyzer(p)
{}

void
arttest::PMTestAnalyzer::analyze(art::Event const&)
{}

DEFINE_ART_MODULE(arttest::PMTestAnalyzer)
