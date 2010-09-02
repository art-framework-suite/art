#ifndef Integration_RunSubRunEventAnalyzer_h
#define Integration_RunSubRunEventAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include <vector>

namespace edmtest {

  class RunSubRunEventAnalyzer : public edm::EDAnalyzer {
  public:

    explicit RunSubRunEventAnalyzer(edm::ParameterSet const& pset);

    virtual ~RunSubRunEventAnalyzer() {}

    virtual void analyze(edm::Event const& event, edm::EventSetup const& es);
    virtual void beginRun(edm::Run const& run, edm::EventSetup const& es);
    virtual void endRun(edm::Run const& run, edm::EventSetup const& es);
    virtual void beginSubRun(edm::SubRun const& lumi, edm::EventSetup const& es);
    virtual void endSubRun(edm::SubRun const& lumi, edm::EventSetup const& es);

  private:

    std::vector<unsigned int> expectedRunLumisEvents_;
    int index_;
    bool verbose_;
  };

}

#endif
