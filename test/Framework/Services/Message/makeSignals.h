#ifndef FWCore_MessageService_test_makeSignals_h
#define FWCore_MessageService_test_makeSignals_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class makeSignals
  : public art::EDAnalyzer
{
public:
  explicit
    makeSignals( art::ParameterSet const & ) { }

  virtual
    ~makeSignals() { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};

}  // namespace edmtest


#endif  // FWCore_MessageService_test_makeSignals_h
