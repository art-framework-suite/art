#ifndef test_Integration_oldtest_RunSubRunEventAnalyzer_h
#define test_Integration_oldtest_RunSubRunEventAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include <vector>

namespace arttest {

  class RunSubRunEventAnalyzer : public art::EDAnalyzer {
  public:

    explicit RunSubRunEventAnalyzer(fhicl::ParameterSet const& pset);

    virtual ~RunSubRunEventAnalyzer() {}

    virtual void analyze(art::Event const& event, art::EventSetup const& es);
    virtual void beginRun(art::Run const& run, art::EventSetup const& es);
    virtual void endRun(art::Run const& run, art::EventSetup const& es);
    virtual void beginSubRun(art::SubRun const& subRun, art::EventSetup const& es);
    virtual void endSubRun(art::SubRun const& subRun, art::EventSetup const& es);

  private:

    std::vector<unsigned int> expectedRunSubRunsEvents_;
    int index_;
    bool verbose_;
  };

}

#endif /* test_Integration_oldtest_RunSubRunEventAnalyzer_h */

// Local Variables:
// mode: c++
// End:
