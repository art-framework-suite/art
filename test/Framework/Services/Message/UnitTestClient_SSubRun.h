#ifndef FWCore_MessageService_test_UnitTestClient_SSubRun_h
#define FWCore_MessageService_test_UnitTestClient_SSubRun_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/MessageLogger/LoggedErrorsSummary.h"
#include "art/Framework/Core/SubRun.h"

#include "art/ParameterSet/ParameterSet.h"

namespace art {
  class ParameterSet;
}


namespace arttest
{

class UTC_SL1
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_SL1( art::ParameterSet const & p)
  {
    identifier = p.getUntrackedParameter<int> ("identifier", 99);
    art::GroupLogStatistics("grouped_cat");
  }

  virtual
    ~UTC_SL1()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  int identifier;
  static bool enableNotYetCalled;
  static int n;
};

class UTC_SL2
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_SL2( art::ParameterSet const & p)
  {
    identifier = p.getUntrackedParameter<int> ("identifier", 98);
  }

  virtual
    ~UTC_SL2()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  int identifier;
  static int n;
};

class UTC_SLUMMARY
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_SLUMMARY( art::ParameterSet const & p)
  {
  }

  virtual
    ~UTC_SLUMMARY()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

  virtual
    void endSubRun ( art::SubRun const & lb
                	    , art::EventSetup 	   const & c
                	    );

private:
};


}  // namespace arttest


#endif  // FWCore_MessageService_test_UnitTestClient_SSubRun_h
