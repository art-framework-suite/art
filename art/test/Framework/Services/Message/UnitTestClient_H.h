#ifndef test_Framework_Services_Message_UnitTestClient_H_h
#define test_Framework_Services_Message_UnitTestClient_H_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"


namespace art {
  class ParameterSet;
}


namespace arttest
{

class UnitTestClient_H
  : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_H( fhicl::ParameterSet const & )
  { }

  virtual
    ~UnitTestClient_H()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_H_h */

// Local Variables:
// mode: c++
// End:
