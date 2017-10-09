#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "cetlib/HorizontalRule.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace arttest {
  class ToyRawInputTester;
}

using strings = std::vector<std::string>;
using std::ostringstream;
using std::string;

using namespace art;

class arttest::ToyRawInputTester : public art::EDAnalyzer {
public:
  explicit ToyRawInputTester(fhicl::ParameterSet const& p)
    : art::EDAnalyzer{p}, fileNames_{p.get<strings>("fileNames")}, pset_{p}
  {
    numFilesExpected_ = fileNames_.size();
    ostringstream expected;
    for (size_t i = 0; i != numFilesExpected_; ++i) {
      expected << "open " << fileNames_[i] << '\n';

      std::vector<std::tuple<int, int, int>> tokens;
      try { // Assume it's a real filename
        fhicl::intermediate_table raw_config;
        cet::filepath_lookup_after1 lookupPolicy(".:");
        fhicl::parse_document(fileNames_[i], lookupPolicy, raw_config);
        fhicl::ParameterSet file_pset;
        make_ParameterSet(raw_config, file_pset);
        assert(file_pset.get_if_present("data", tokens));
      }
      catch (...) { // Attempt to read the parameter set by that name instead.
        assert(p.get_if_present(fileNames_[i], tokens));
      }
      RunNumber_t currentRun = -1;
      SubRunNumber_t currentSubRun = -1;
      for (auto const& token : tokens) {
        int newRun, newSubRun, newEvent;
        std::tie(newRun, newSubRun, newEvent) = token;
        if (newRun != -1) {
          ++numRunsExpected_;
          currentRun = newRun;
          expected << "begin " << RunID(currentRun) << '\n';
        }
        if (newSubRun != -1) {
          ++numSubRunsExpected_;
          currentSubRun = newSubRun;
          expected << "begin " << SubRunID(currentRun, currentSubRun) << '\n';
        }
        if (newEvent != -1) {
          ++numEventsExpected_;
          expected << "event " << EventID(currentRun, currentSubRun, newEvent)
                   << '\n';
        }
      }
    }
    expectedMessage_ = expected.str();
  }

  void
  respondToOpenInputFile(art::FileBlock const& fb) override
  {
    ++numFilesSeen_;
    messages_ << "open " << fb.fileName() << '\n';
  }

  void
  beginRun(art::Run const& r) override
  {
    ++numRunsSeen_;
    messages_ << "begin " << r.id() << '\n';
  }

  void
  beginSubRun(art::SubRun const& sr) override
  {
    ++numSubRunsSeen_;
    messages_ << "begin " << sr.id() << '\n';
  }

  void
  analyze(art::Event const& e) override
  {
    ++numEventsSeen_;
    messages_ << "event " << e.id() << '\n';
  }

  void
  endJob() override
  {
    cet::HorizontalRule constexpr rule{72};
    std::cerr << rule('-') << '\n'
              << expectedMessage_ << rule('-') << '\n'
              << rule('+') << '\n'
              << messages_.str() << rule('+') << '\n';
    assert(numFilesSeen_ == numFilesExpected_);
    assert(numRunsSeen_ == numRunsExpected_);
    assert(numSubRunsSeen_ == numSubRunsExpected_);
    assert(numEventsSeen_ == numEventsExpected_);
    assert(expectedMessage_ == messages_.str());
  }

private:
  size_t numEventsSeen_{};
  size_t numEventsExpected_{};
  size_t numSubRunsSeen_{};
  size_t numSubRunsExpected_{};
  size_t numRunsSeen_{};
  size_t numRunsExpected_{};
  size_t numFilesSeen_{};
  size_t numFilesExpected_{};
  strings fileNames_;
  fhicl::ParameterSet pset_;
  ostringstream messages_{};
  string expectedMessage_{};
};

DEFINE_ART_MODULE(arttest::ToyRawInputTester)
