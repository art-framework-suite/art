// This test module will look for history information in event data.

#include <iostream>
#include <vector>

#include "art/Persistency/Common/Handle.h"
#include "test/TestObjects/ToyProducts.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/ParameterSet/InputTag.h"
#include "art/ParameterSet/ParameterSet.h"

namespace arttest
{
  class TestHistoryKeeping : public art::EDAnalyzer
  {
  public:

    explicit TestHistoryKeeping(art::ParameterSet const& pset);
    virtual ~TestHistoryKeeping();

    virtual void analyze(art::Event const& e, art::EventSetup const&);

    virtual void beginRun(art::Run const& r, art::EventSetup const&);
    virtual void endRun(art::Run const& r, art::EventSetup const&);

  private:
    std::vector<std::string> expectedProcesses_;
    int   numberOfExpectedHLTProcessesInEachRun_;
  }; // class TestHistoryKeeping



  //--------------------------------------------------------------------
  //
  // Implementation details
  //--------------------------------------------------------------------

  TestHistoryKeeping::TestHistoryKeeping(art::ParameterSet const& pset) :
    expectedProcesses_(pset.getParameter<std::vector<std::string> >("expected_processes")),
    numberOfExpectedHLTProcessesInEachRun_(pset.getParameter<int>("number_of_expected_HLT_processes_for_each_run"))
  {
    // Nothing to do.
  }

  TestHistoryKeeping::~TestHistoryKeeping() {}

  void
  TestHistoryKeeping::beginRun(art::Run const&, art::EventSetup const&)
  {
    // At begin run, we're looking at, make sure we can get at the
    // parameter sets for any HLT processing.
  }

  void
  TestHistoryKeeping::analyze(art::Event const& ev, art::EventSetup const&)
  {
    for (std::vector<std::string>::const_iterator
	   i = expectedProcesses_.begin(),
	   e = expectedProcesses_.end();
	 i != e;
	 ++i)
      {
	art::ParameterSet ps;
	assert(ev.getProcessParameterSet(*i, ps));
	assert(!ps.empty());
	assert(ps.getParameter<std::string>("process_name") == *i);
      }
  }

  void
  TestHistoryKeeping::endRun(art::Run const&, art::EventSetup const& )
  {
    // Nothing to do.
  }

} // arttest

using arttest::TestHistoryKeeping;
DEFINE_FWK_MODULE(TestHistoryKeeping);
