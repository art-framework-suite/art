#ifndef test_Framework_Services_Message_UnitTestClient_N_h
#define test_Framework_Services_Message_UnitTestClient_N_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_N
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_N( fhicl::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_N()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_N_h */

// Local Variables:
// mode: c++
// End:
