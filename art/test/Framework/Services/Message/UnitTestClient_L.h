#ifndef test_Framework_Services_Message_UnitTestClient_L_h
#define test_Framework_Services_Message_UnitTestClient_L_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_L : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_L( fhicl::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_L()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};

class UnitTestClient_L1 : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_L1( fhicl::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_L1()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest




#endif /* test_Framework_Services_Message_UnitTestClient_L_h */

// Local Variables:
// mode: c++
// End:
