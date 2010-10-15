#ifndef FWCore_MessageService_test_UnitTestClient_P_h
#define FWCore_MessageService_test_UnitTestClient_P_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include "art/ParameterSet/ParameterSet.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_P
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_P( art::ParameterSet const & p)
    : useLogFlush(true)
    , queueFillers(1)
  {
    useLogFlush  = p.getUntrackedParameter<bool>("useLogFlush",  useLogFlush);
    queueFillers = p.getUntrackedParameter<int> ("queueFillers", queueFillers);
  }

  virtual
    ~UnitTestClient_P()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  bool useLogFlush;
  int  queueFillers;
};


}  // namespace arttest


#endif  // FWCore_MessageService_test_UnitTestClient_P_h
