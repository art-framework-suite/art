#ifndef FWCore_MessageService_test_UnitTestClient_S_h
#define FWCore_MessageService_test_UnitTestClient_S_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/MessageLogger/LoggedErrorsSummary.h"

#include "art/ParameterSet/ParameterSet.h"

namespace art {
  class ParameterSet;
}


namespace arttest
{

class UTC_S1
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_S1( art::ParameterSet const & p)
  {
    identifier = p.getUntrackedParameter<int> ("identifier", 99);
    art::GroupLogStatistics("grouped_cat");
  }

  virtual
    ~UTC_S1()
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

class UTC_S2
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_S2( art::ParameterSet const & p)
  {
    identifier = p.getUntrackedParameter<int> ("identifier", 98);
  }

  virtual
    ~UTC_S2()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  int identifier;
  static int n;
};

class UTC_SUMMARY
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_SUMMARY( art::ParameterSet const & p)
  {
  }

  virtual
    ~UTC_SUMMARY()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
};


}  // namespace arttest


#endif  // FWCore_MessageService_test_UnitTestClient_S_h
