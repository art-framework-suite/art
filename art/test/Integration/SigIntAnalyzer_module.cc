////////////////////////////////////////////////////////////////////////
// Class:       SigIntAnalyzer
// Module Type: analyzer
// File:        SigIntAnalyzer_module.cc
//
// Generated at Wed Jul 17 11:09:19 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <signal.h>
#include <string>
#include <unistd.h>

namespace art {
  namespace test {
    class SigIntAnalyzer;
  }
}

class art::test::SigIntAnalyzer : public EDAnalyzer {
public:
  struct Config {
    fhicl::Atom<std::string> signal{
      fhicl::Name{"signal"},
      fhicl::Comment{"Job-ending signal to be emitted.  Can choose from\n"
                     "SIGUSR2, SIGTERM, SIGQUIT, or SIGINT."}};
  };
  using Parameters = EDAnalyzer::Table<Config>;

  explicit SigIntAnalyzer(Parameters const& p);

  void analyze(Event const&) override;

private:
  int signal_;
};

art::test::SigIntAnalyzer::SigIntAnalyzer(Parameters const& p) : EDAnalyzer{p}
{
  auto const signal = p().signal();
  if (signal == "SIGUSR2") {
    signal_ = SIGUSR2;
  } else if (signal == "SIGTERM") {
    signal_ = SIGTERM;
  } else if (signal == "SIGQUIT") {
    signal_ = SIGQUIT;
  } else if (signal == "SIGINT") {
    signal_ = SIGINT;
  } else {
    throw art::Exception{art::errors::Configuration}
      << "Incorrect signal string specified.  Please select from:\n"
      << "  SIGUSR2,\n"
      << "  SIGTERM,\n"
      << "  SIGQUIT, or\n"
      << "  SIGINT.";
  }
}

void
art::test::SigIntAnalyzer::analyze(Event const&)
{
  kill(getpid(), signal_);
}

DEFINE_ART_MODULE(art::test::SigIntAnalyzer)
