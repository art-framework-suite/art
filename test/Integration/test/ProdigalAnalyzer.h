#ifndef Integration_ProdigalAnalyzer_h
#define Integration_ProdigalAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace edmtest {

  class ProdigalAnalyzer : public edm::EDAnalyzer {
  public:
    explicit ProdigalAnalyzer(edm::ParameterSet const& pset);
    virtual ~ProdigalAnalyzer() {}
    virtual void analyze(edm::Event const& e, edm::EventSetup const& c);
  };

}

#endif
