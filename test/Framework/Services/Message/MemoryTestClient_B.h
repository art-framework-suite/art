#ifndef test_Framework_Services_Message_MemoryTestClient_B_h
#define test_Framework_Services_Message_MemoryTestClient_B_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include <vector>

// MemoryTestClient_B is used for testing JObReprt outputs from
// the MemoryService

namespace art {
  class ParameterSet;
}


namespace arttest
{

class MemoryTestClient_B
  : public art::EDAnalyzer
{
public:
  explicit
    MemoryTestClient_B( fhicl::ParameterSet const & );

  virtual
    ~MemoryTestClient_B()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  static int nevent;
  std::vector<double> memoryPattern;
  void initializeMemoryPattern(int pattern);
  double vsize;
  char* last_Allocation;
};


}  // arttest


#endif /* test_Framework_Services_Message_MemoryTestClient_B_h */

// Local Variables:
// mode: c++
// End:
