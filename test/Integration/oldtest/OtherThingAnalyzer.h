#ifndef test_Integration_oldtest_OtherThingAnalyzer_h
#define test_Integration_oldtest_OtherThingAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace arttest {

  class OtherThingAnalyzer : public art::EDAnalyzer {
  public:

    explicit OtherThingAnalyzer(fhicl::ParameterSet const& pset);

    virtual ~OtherThingAnalyzer() {}

    virtual void analyze(art::Event const& e, art::EventSetup const& c);

    void doit(art::Event const& event, std::string const& label);

  private:
    bool thingWasDropped_;
  };

}

#endif /* test_Integration_oldtest_OtherThingAnalyzer_h */

// Local Variables:
// mode: c++
// End:
