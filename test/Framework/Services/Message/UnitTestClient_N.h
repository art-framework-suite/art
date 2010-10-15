#ifndef FWCore_MessageService_test_UnitTestClient_N_h
#define FWCore_MessageService_test_UnitTestClient_N_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_N
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_N( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_N()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_UnitTestClient_N_h
