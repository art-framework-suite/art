#ifndef FWCore_MessageService_test_UnitTestClient_I_h
#define FWCore_MessageService_test_UnitTestClient_I_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_I
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_I( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_I()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_UnitTestClient_I_h
