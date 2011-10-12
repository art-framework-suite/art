#ifndef test_Framework_Services_Message_UnitTestClient_J_h
#define test_Framework_Services_Message_UnitTestClient_J_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest {

  class UnitTestClient_J
      : public art::EDAnalyzer {
  public:
    explicit
    UnitTestClient_J(fhicl::ParameterSet const &)
    { }

    virtual
    ~UnitTestClient_J()
    { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

  private:
  };


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_J_h */

// Local Variables:
// mode: c++
// End:
