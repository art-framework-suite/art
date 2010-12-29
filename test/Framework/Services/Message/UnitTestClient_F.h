#ifndef FWCore_MessageService_test_UnitTestClient_F_h
#define FWCore_MessageService_test_UnitTestClient_F_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_F
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_F( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_F()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif
