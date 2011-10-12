#ifndef test_Framework_Services_Message_makeSignals_h
#define test_Framework_Services_Message_makeSignals_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest {

  class makeSignals
      : public art::EDAnalyzer {
  public:
    explicit
    makeSignals(fhicl::ParameterSet const &) { }

    virtual
    ~makeSignals() { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

  private:
  };

}  // arttest


#endif /* test_Framework_Services_Message_makeSignals_h */

// Local Variables:
// mode: c++
// End:
