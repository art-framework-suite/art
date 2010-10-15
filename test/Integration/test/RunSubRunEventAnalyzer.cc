
#include "FWCore/Integration/test/RunSubRunEventAnalyzer.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/ParameterSet/ParameterSet.h"

#include <cassert>
#include <iostream>

namespace edmtest {

  RunSubRunEventAnalyzer::RunSubRunEventAnalyzer(art::ParameterSet const& pset) :
    expectedRunSubRunsEvents_(pset.getUntrackedParameter<std::vector<unsigned int> >("expectedRunSubRunEvents", std::vector<unsigned int>())),
    index_(0),
    verbose_(pset.getUntrackedParameter<bool>("verbose", false)) {
  }

  void RunSubRunEventAnalyzer::analyze(art::Event const& event, art::EventSetup const& es) {

    if (verbose_) {
      art::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << event.run() << ", "
                                       << event.subRun() << ", "
                                       << event.id().event();
    }

    if ((index_ + 2U) < expectedRunSubRunsEvents_.size()) {
      assert(expectedRunSubRunsEvents_[index_++] == event.run());
      assert(expectedRunSubRunsEvents_[index_++] == event.subRun());
      assert(expectedRunSubRunsEvents_[index_++] == event.id().event());
    }
  }

  void RunSubRunEventAnalyzer::beginRun(art::Run const& run, art::EventSetup const& es) {

    if (verbose_) {
      art::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << run.run() << ", "
                                       << 0 << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunSubRunsEvents_.size()) {
      assert(expectedRunSubRunsEvents_[index_++] == run.run());
      assert(expectedRunSubRunsEvents_[index_++] == 0);
      assert(expectedRunSubRunsEvents_[index_++] == 0);
    }
  }

  void RunSubRunEventAnalyzer::endRun(art::Run const& run, art::EventSetup const& es) {

    if (verbose_) {
      art::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << run.run() << ", "
                                       << 0 << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunSubRunsEvents_.size()) {
      assert(expectedRunSubRunsEvents_[index_++] == run.run());
      assert(expectedRunSubRunsEvents_[index_++] == 0);
      assert(expectedRunSubRunsEvents_[index_++] == 0);
    }
  }

  void RunSubRunEventAnalyzer::beginSubRun(art::SubRun const& subRun, art::EventSetup const& es) {

    if (verbose_) {
      art::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << subRun.run() << ", "
                                       << subRun.subRun() << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunSubRunsEvents_.size()) {
      assert(expectedRunSubRunsEvents_[index_++] == subRun.run());
      assert(expectedRunSubRunsEvents_[index_++] == subRun.subRun());
      assert(expectedRunSubRunsEvents_[index_++] == 0);
    }
  }

  void RunSubRunEventAnalyzer::endSubRun(art::SubRun const& subRun, art::EventSetup const& es) {

    if (verbose_) {
      art::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << subRun.run() << ", "
                                       << subRun.subRun() << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunSubRunsEvents_.size()) {
      assert(expectedRunSubRunsEvents_[index_++] == subRun.run());
      assert(expectedRunSubRunsEvents_[index_++] == subRun.subRun());
      assert(expectedRunSubRunsEvents_[index_++] == 0);
    }
  }
}

using edmtest::RunSubRunEventAnalyzer;
DEFINE_FWK_MODULE(RunSubRunEventAnalyzer);
