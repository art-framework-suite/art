#ifndef FWCore_MessageService_test_UnitTestClient_R_h
#define FWCore_MessageService_test_UnitTestClient_R_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_R
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_R( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_R()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_UnitTestClient_A_h
