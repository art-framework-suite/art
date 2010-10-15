#ifndef Integration_ProdigalAnalyzer_h
#define Integration_ProdigalAnalyzer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace arttest {

  class ProdigalAnalyzer : public art::EDAnalyzer {
  public:
    explicit ProdigalAnalyzer(art::ParameterSet const& pset);
    virtual ~ProdigalAnalyzer() {}
    virtual void analyze(art::Event const& e, art::EventSetup const& c);
  };

}

#endif
