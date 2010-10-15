#ifndef FWCore_MessageService_test_UnitTestClient_L_h
#define FWCore_MessageService_test_UnitTestClient_L_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace art {
  class ParameterSet;
}


namespace edmtest
{

class UnitTestClient_L : public art::EDAnalyzer
{
public:
  explicit
    UnitTestClient_L( art::ParameterSet const & )
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
    UnitTestClient_L1( art::ParameterSet const & )
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


}  // namespace edmtest




#endif  // FWCore_MessageService_test_UnitTestClient_L_h
