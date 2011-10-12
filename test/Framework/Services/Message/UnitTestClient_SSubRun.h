#ifndef test_Framework_Services_Message_UnitTestClient_SSubRun_h
#define test_Framework_Services_Message_UnitTestClient_SSubRun_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "messagefacility/MessageLogger/LoggedErrorsSummary.h"
#include "art/Framework/Principal/SubRun.h"

#include "fhiclcpp/ParameterSet.h"

namespace art {
  class ParameterSet;
}


namespace arttest {

  class UTC_SL1
      : public art::EDAnalyzer {
  public:
    explicit
    UTC_SL1(fhicl::ParameterSet const & p) {
      identifier = p.get<int> ("identifier", 99);
      art::GroupLogStatistics("grouped_cat");
    }

    virtual
    ~UTC_SL1()
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

  class UTC_SL2
      : public art::EDAnalyzer {
  public:
    explicit
    UTC_SL2(fhicl::ParameterSet const & p) {
      identifier = p.get<int> ("identifier", 98);
    }

    virtual
    ~UTC_SL2()
    { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

  private:
    int identifier;
    static int n;
  };

  class UTC_SLUMMARY
      : public art::EDAnalyzer {
  public:
    explicit
    UTC_SLUMMARY(fhicl::ParameterSet const & p) {
    }

    virtual
    ~UTC_SLUMMARY()
    { }

    virtual
    void analyze(art::Event      const & e
                 , art::EventSetup const & c
                );

    virtual
    void endSubRun(art::SubRun const & sr
                   , art::EventSetup      const & c
                  );

  private:
  };


}  // arttest


#endif /* test_Framework_Services_Message_UnitTestClient_SSubRun_h */

// Local Variables:
// mode: c++
// End:
