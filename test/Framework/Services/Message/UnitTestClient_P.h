#ifndef test_Framework_Services_Message_UnitTestClient_P_h
#define test_Framework_Services_Message_UnitTestClient_P_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include "fhiclcpp/ParameterSet.h"


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
    UnitTestClient_P( fhicl::ParameterSet const & p)
    : useLogFlush(true)
    , queueFillers(1)
  {
    useLogFlush  = p.get<bool>("useLogFlush",  useLogFlush);
    queueFillers = p.get<int> ("queueFillers", queueFillers);
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


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_P_h */

// Local Variables:
// mode: c++
// End:
