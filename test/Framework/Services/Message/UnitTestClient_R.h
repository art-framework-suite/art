#ifndef test_Framework_Services_Message_UnitTestClient_R_h
#define test_Framework_Services_Message_UnitTestClient_R_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest {

  class UnitTestClient_R
      : public art::EDAnalyzer {
  public:
    explicit
    UnitTestClient_R(fhicl::ParameterSet const &)
    { }

    virtual
    ~UnitTestClient_R()
    { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

  private:
  };


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_R_h */

// Local Variables:
// mode: c++
// End:
