#ifndef FWCore_MessageService_test_UnitTestClient_E_h
#define FWCore_MessageService_test_UnitTestClient_E_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_E
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_E( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_E()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_UnitTestClient_E_h
