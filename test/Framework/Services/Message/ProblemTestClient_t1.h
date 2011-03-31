#ifndef test_Framework_Services_Message_ProblemTestClient_t1_h
#define test_Framework_Services_Message_ProblemTestClient_t1_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class ProblemTestClient_t1
  : public art::EDAnalyzer
{
public:
  explicit
    ProblemTestClient_t1( fhicl::ParameterSet const & )
  { }

  virtual
    ~ProblemTestClient_t1()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif /* test_Framework_Services_Message_ProblemTestClient_t1_h */

// Local Variables:
// mode: c++
// End:
