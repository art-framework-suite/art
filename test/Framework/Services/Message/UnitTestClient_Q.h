#ifndef test_Framework_Services_Message_UnitTestClient_Q_h
#define test_Framework_Services_Message_UnitTestClient_Q_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/MessageLogger/MessageLogger.h"

#include "art/ParameterSet/ParameterSet.h"

namespace art {
  class ParameterSet;
}


namespace arttest
{

class UTC_Q1
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_Q1( art::ParameterSet const & p)
  {
    identifier = p.getUntrackedParameter<int> ("identifier", 99);
    art::GroupLogStatistics("timer");  // these lines would normally be in
    art::GroupLogStatistics("trace");  // whatever service knows that
    				       // timer and trace should be groupd
				       // by moduels for statistics
  }

  virtual
    ~UTC_Q1()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  int identifier;
};

class UTC_Q2
  : public art::EDAnalyzer
{
public:
  explicit
    UTC_Q2( art::ParameterSet const & p)
  {
    identifier = p.getUntrackedParameter<int> ("identifier", 98);
  }

  virtual
    ~UTC_Q2()
  { }

  virtual
    void analyze( art::Event      const & e
                , art::EventSetup const & c
                );

private:
  int identifier;
};


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_Q_h */

// Local Variables:
// mode: c++
// End:
