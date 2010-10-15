#ifndef FWCore_MessageService_test_UnitTestClient_K_h
#define FWCore_MessageService_test_UnitTestClient_K_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_K
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_K( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_K()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace arttest


#endif  // FWCore_MessageService_test_UnitTestClient_K_h
