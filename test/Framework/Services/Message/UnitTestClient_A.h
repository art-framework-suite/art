#ifndef test_Framework_Services_Message_UnitTestClient_A_h
#define test_Framework_Services_Message_UnitTestClient_A_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_A
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_A( art::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_A()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_A_h */

// Local Variables:
// mode: c++
// End:
