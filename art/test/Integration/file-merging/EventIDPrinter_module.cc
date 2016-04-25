#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>

namespace {
  struct Config{};
}

namespace arttest {

  class EventIDPrinter : public art::EDAnalyzer {
  public:

    using Parameters = art::EDAnalyzer::Table<Config>;

    EventIDPrinter(Parameters const& ps)
      : art::EDAnalyzer{ps}
    {}

  private:

    void analyze(art::Event const& e) override
    {
      std::cout << e.id() << std::endl;
    }

  };

}

DEFINE_ART_MODULE(arttest::EventIDPrinter)
