#ifndef FWCore_MessageService_test_UnitTestClient_J_h
#define FWCore_MessageService_test_UnitTestClient_J_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_J
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_J( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_J()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_UnitTestClient_J_h
