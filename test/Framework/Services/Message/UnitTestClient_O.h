#ifndef FWCore_MessageService_test_UnitTestClient_O_h
#define FWCore_MessageService_test_UnitTestClient_O_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_O
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_O( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_O()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_UnitTestClient_O_h
