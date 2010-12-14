#ifndef Integration_OtherThingAnalyzer_h
#define Integration_OtherThingAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace arttest {

  class OtherThingAnalyzer : public art::EDAnalyzer {
  public:

    explicit OtherThingAnalyzer(art::ParameterSet const& pset);

    virtual ~OtherThingAnalyzer() {}

    virtual void analyze(art::Event const& e, art::EventSetup const& c);

    void doit(art::Event const& event, std::string const& label);

  private:
    bool thingWasDropped_;
  };

}

#endif
