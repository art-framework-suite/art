#ifndef test_Framework_Services_Message_UnitTestClient_M_h
#define test_Framework_Services_Message_UnitTestClient_M_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_M
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_M( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_M()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_M_h */

// Local Variables:
// mode: c++
// End:
