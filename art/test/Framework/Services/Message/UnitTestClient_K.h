#ifndef art_test_Framework_Services_Message_UnitTestClient_K_h
#define art_test_Framework_Services_Message_UnitTestClient_K_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_K
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_K( fhicl::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_K()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif /* art_test_Framework_Services_Message_UnitTestClient_K_h */

// Local Variables:
// mode: c++
// End:
