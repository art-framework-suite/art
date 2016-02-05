#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/types/Atom.h"

namespace {

  struct Config {
    fhicl::Atom<bool> doThrow { fhicl::Name("throw") };
  };

}

namespace arttest {

  class ThrowingAnalyzer : public art::EDAnalyzer {
    bool doThrow_{false};
  public:

    using Parameters = art::EDAnalyzer::Table<Config>;

    ThrowingAnalyzer(Parameters const& c)
      : EDAnalyzer(c)
      , doThrow_( c().doThrow() )
    {
      if ( doThrow_ )
        throw art::Exception(art::errors::Configuration, "Throwing analyzer ctor");
    }

    void analyze(art::Event const&) override {}

  };

}

DEFINE_ART_MODULE(arttest::ThrowingAnalyzer)
