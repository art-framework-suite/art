#ifndef test_Integration_oldtest_ProdigalAnalyzer_h
#define test_Integration_oldtest_ProdigalAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace arttest {

  class ProdigalAnalyzer : public art::EDAnalyzer {
  public:
    explicit ProdigalAnalyzer(fhicl::ParameterSet const& pset);
    virtual ~ProdigalAnalyzer() {}
    virtual void analyze(art::Event const& e, art::EventSetup const& c);
  };

}

#endif /* test_Integration_oldtest_ProdigalAnalyzer_h */

// Local Variables:
// mode: c++
// End:
