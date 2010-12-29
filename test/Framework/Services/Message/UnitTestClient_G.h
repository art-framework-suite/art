#ifndef FWCore_MessageService_test_UnitTestClient_G_h
#define FWCore_MessageService_test_UnitTestClient_G_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
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


}  // arttest


#endif
