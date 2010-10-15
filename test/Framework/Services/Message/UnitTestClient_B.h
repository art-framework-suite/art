#ifndef FWCore_MessageService_test_UnitTestClient_B_h
#define FWCore_MessageService_test_UnitTestClient_B_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

// UnitTestClient_B is used for testing LogStatistics and the reset behaviors
// of statistics destinations.

namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_B
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_B( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_B()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  static int nevent;
};


}  // namespace arttest


#endif  // FWCore_MessageService_test_UnitTestClient_B_h
