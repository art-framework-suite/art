////////////////////////////////////////////////////////////////////////
// Class:       PausingAnalyzer
// Module Type: analyzer
// File:        PausingAnalyzer_module.cc
//
// Generated at Wed Jul 17 11:09:19 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

extern "C" {
#include <unistd.h>
}

#include <iostream>

namespace arttest {
  class PausingAnalyzer;
}

class arttest::PausingAnalyzer : public art::EDAnalyzer {
public:
  explicit PausingAnalyzer(fhicl::ParameterSet const & p);

  void analyze(art::Event const & e) override;

private:
  unsigned int pauseTime_;
};

arttest::PausingAnalyzer::PausingAnalyzer(fhicl::ParameterSet const & p)
:
  art::EDAnalyzer(p),
  pauseTime_(p.get<unsigned int>("pauseTime", 5))
{
}

void arttest::PausingAnalyzer::analyze(art::Event const &)
{
  std::cerr << ">> Pausing for " << pauseTime_
            << " seconds.\n";
  sleep(pauseTime_);
  std::cerr << ">> Pause complete.\n";
}

DEFINE_ART_MODULE(arttest::PausingAnalyzer)
