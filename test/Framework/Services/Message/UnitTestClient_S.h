#ifndef test_Framework_Services_Message_UnitTestClient_S_h
#define test_Framework_Services_Message_UnitTestClient_S_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "messagefacility/MessageLogger/LoggedErrorsSummary.h"

#include "fhiclcpp/ParameterSet.h"

namespace art {
  class ParameterSet;
}


namespace arttest {

  class UTC_S1
      : public art::EDAnalyzer {
  public:
    explicit
    UTC_S1(fhicl::ParameterSet const & p) {
      identifier = p.get<int> ("identifier", 99);
      art::GroupLogStatistics("grouped_cat");
    }

    virtual
    ~UTC_S1()
    { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

  private:
    int identifier;
    static bool enableNotYetCalled;
    static int n;
  };

  class UTC_S2
      : public art::EDAnalyzer {
  public:
    explicit
    UTC_S2(fhicl::ParameterSet const & p) {
      identifier = p.get<int> ("identifier", 98);
    }

    virtual
    ~UTC_S2()
    { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

  private:
    int identifier;
    static int n;
  };

  class UTC_SUMMARY
      : public art::EDAnalyzer {
  public:
    explicit
    UTC_SUMMARY(fhicl::ParameterSet const & p) {
    }

    virtual
    ~UTC_SUMMARY()
    { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

  private:
  };

}  // arttest

#endif /* test_Framework_Services_Message_UnitTestClient_S_h */

// Local Variables:
// mode: c++
// End:
