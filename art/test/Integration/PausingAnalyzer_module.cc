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
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <chrono>
#include <thread>

namespace art {
  namespace test {
    class PausingAnalyzer;
  }
}

class art::test::PausingAnalyzer : public EDAnalyzer {
public:

  struct Config {
    fhicl::Atom<unsigned> pauseTime {fhicl::Name{"pauseTime"}, fhicl::Comment{"Pause time is in seconds."}, 5u};
  };
  using Parameters = EDAnalyzer::Table<Config>;

  explicit PausingAnalyzer(Parameters const& p);

  void analyze(Event const&) override;

private:
  std::chrono::seconds pauseTime_;
};

art::test::PausingAnalyzer::PausingAnalyzer(Parameters const & p)
  : EDAnalyzer{p},
    pauseTime_{p().pauseTime()}
{
}

void art::test::PausingAnalyzer::analyze(Event const&)
{
  std::cerr << ">> Pausing for " << pauseTime_.count()
            << " seconds.\n";
  std::this_thread::sleep_for(pauseTime_);
  std::cerr << ">> Pause complete.\n";
}

DEFINE_ART_MODULE(art::test::PausingAnalyzer)
