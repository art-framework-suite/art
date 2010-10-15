#ifndef Integration_RunSubRunEventAnalyzer_h
#define Integration_RunSubRunEventAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include <vector>

namespace edmtest {

  class RunSubRunEventAnalyzer : public art::EDAnalyzer {
  public:

    explicit RunSubRunEventAnalyzer(art::ParameterSet const& pset);

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

#endif
