#ifndef FWCore_MessageService_test_UnitTestClient_D_h
#define FWCore_MessageService_test_UnitTestClient_D_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_D
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_D( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_D()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif
