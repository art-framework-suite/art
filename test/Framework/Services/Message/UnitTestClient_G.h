#ifndef FWCore_MessageService_test_UnitTestClient_G_h
#define FWCore_MessageService_test_UnitTestClient_G_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_G
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_G( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_G()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_UnitTestClient_A_h
