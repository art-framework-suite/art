////////////////////////////////////////////////////////////////////////
// Class:       DummyAnalyzer
// Plugin Type: analyzer (art v2_06_03)
// File:        DummyAnalyzer_module.cc
//
// Generated at Fri May 19 09:54:47 2017 by Kyle Knoepfel using cetskelgen
// from cetlib version v2_03_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"

namespace art {
  class Event;
  namespace test {
    class DummyAnalyzer;
  }
} // namespace art

class art::test::DummyAnalyzer : public EDAnalyzer {
public:
  struct Config {
  };
  using Parameters = EDAnalyzer::Table<Config>;
  explicit DummyAnalyzer(Parameters const& p) : EDAnalyzer{p} {}

private:
  void
  analyze(Event const&) override
  {}
};

DEFINE_ART_MODULE(art::test::DummyAnalyzer)
