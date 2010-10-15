#ifndef FWCore_MessageService_test_MemoryTestClient_A_h
#define FWCore_MessageService_test_MemoryTestClient_A_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include <vector>

// MemoryTestClient_A is used for testing JObReprt outputs from
// the MemoryService

namespace art {
  class ParameterSet;
}


namespace edmtest
{

class MemoryTestClient_A
  : public art::EDAnalyzer
{
public:
  explicit
    MemoryTestClient_A( art::ParameterSet const & );

  virtual
    ~MemoryTestClient_A()
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
  char* last_allocation;
};


}  // namespace edmtest


#endif  // FWCore_MessageService_test_MemoryTestClient_A_h
