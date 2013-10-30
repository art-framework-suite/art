
/*----------------------------------------------------------------------

 EDAnalyzer for testing History class and history tracking mechanism.

----------------------------------------------------------------------*/

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ParameterSetID.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h"

using namespace art;

namespace arttest {

  class HistoryAnalyzer : public EDAnalyzer {
  public:

    explicit HistoryAnalyzer(const ParameterSet& params);
    void analyze(const Event& event, EventSetup const&);
    void endJob();

  private:
    int pass_;
    int eventCount_;
    int expectedCount_;
    ParameterSetID emptyID_;
    ParameterSetID outputConfigID_;
  };

  HistoryAnalyzer::HistoryAnalyzer(const ParameterSet& params) :
    EDAnalyzer(params),
    pass_(params.get<int>("historySize")),
    eventCount_(0),
    expectedCount_(params.get<int>("expectedCount")),
    emptyID_(ParameterSet().id()),
    outputConfigID_()
  {
    ParameterSet temp;
    typedef std::vector<std::string> vstring;
    vstring wanted_paths(1, "f55");
    temp.addParameter<std::vector<std::string> >("SelectEvents", wanted_paths);
    outputConfigID_ = temp.id();
  }

  void
  HistoryAnalyzer::analyze(const Event& event, EventSetup const&)
  {
    History const& h = event.history();
    assert(h.size() == static_cast<size_t>(pass_ - 1));

    assert(h.getEventSelectionID(0) == emptyID_);
    assert(h.getEventSelectionID(1) == outputConfigID_);
    assert(h.getEventSelectionID(2) == emptyID_);
    ++eventCount_;
  }

  void
  HistoryAnalyzer::endJob()
  {
    std::cout << "Expected count is: " << expectedCount_ << std::endl;
    std::cout << "Event count is:    " << eventCount_ << std::endl;
    assert(eventCount_ == expectedCount_);
  }
}

using arttest::HistoryAnalyzer;
DEFINE_ART_MODULE(HistoryAnalyzer)
