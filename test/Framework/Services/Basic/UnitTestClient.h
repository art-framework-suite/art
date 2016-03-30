#ifndef test_Framework_Services_Basic_UnitTestClient_h
#define test_Framework_Services_Basic_UnitTestClient_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient : public art::EDAnalyzer
{
public:
  explicit UnitTestClient( fhicl::ParameterSet const & p);
  void analyze( art::Event const & e) override;

private:
};


}  // arttest


#endif /* test_Framework_Services_Basic_UnitTestClient_h */

// Local Variables:
// mode: c++
// End:
