#ifndef FWCore_MessageService_test_UnitTestClient_C_h
#define FWCore_MessageService_test_UnitTestClient_C_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_C
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_C( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_C()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace arttest


#endif  // FWCore_MessageService_test_UnitTestClient_A_h
