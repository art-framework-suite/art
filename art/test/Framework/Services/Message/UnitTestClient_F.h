#ifndef test_Framework_Services_Message_UnitTestClient_F_h
#define test_Framework_Services_Message_UnitTestClient_F_h

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
    UnitTestClient_F( fhicl::ParameterSet const & )
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


#endif /* test_Framework_Services_Message_UnitTestClient_F_h */

// Local Variables:
// mode: c++
// End:
