
#include "FWCore/Integration/test/RunSubRunEventAnalyzer.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/ParameterSet/ParameterSet.h"

#include <cassert>
#include <iostream>

namespace edmtest {

  RunSubRunEventAnalyzer::RunSubRunEventAnalyzer(edm::ParameterSet const& pset) :
    expectedRunLumisEvents_(pset.getUntrackedParameter<std::vector<unsigned int> >("expectedRunLumiEvents", std::vector<unsigned int>())),
    index_(0),
    verbose_(pset.getUntrackedParameter<bool>("verbose", false)) {
  }

  void RunSubRunEventAnalyzer::analyze(edm::Event const& event, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << event.run() << ", "
                                       << event.luminosityBlock() << ", "
                                       << event.id().event();
    }

    if ((index_ + 2U) < expectedRunLumisEvents_.size()) {
      assert(expectedRunLumisEvents_[index_++] == event.run());
      assert(expectedRunLumisEvents_[index_++] == event.luminosityBlock());
      assert(expectedRunLumisEvents_[index_++] == event.id().event());
    }
  }

  void RunSubRunEventAnalyzer::beginRun(edm::Run const& run, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << run.run() << ", "
                                       << 0 << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunLumisEvents_.size()) {
      assert(expectedRunLumisEvents_[index_++] == run.run());
      assert(expectedRunLumisEvents_[index_++] == 0);
      assert(expectedRunLumisEvents_[index_++] == 0);
    }
  }

  void RunSubRunEventAnalyzer::endRun(edm::Run const& run, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << run.run() << ", "
                                       << 0 << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunLumisEvents_.size()) {
      assert(expectedRunLumisEvents_[index_++] == run.run());
      assert(expectedRunLumisEvents_[index_++] == 0);
      assert(expectedRunLumisEvents_[index_++] == 0);
    }
  }

  void RunSubRunEventAnalyzer::beginSubRun(edm::SubRun const& lumi, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << lumi.run() << ", "
                                       << lumi.luminosityBlock() << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunLumisEvents_.size()) {
      assert(expectedRunLumisEvents_[index_++] == lumi.run());
      assert(expectedRunLumisEvents_[index_++] == lumi.luminosityBlock());
      assert(expectedRunLumisEvents_[index_++] == 0);
    }
  }

  void RunSubRunEventAnalyzer::endSubRun(edm::SubRun const& lumi, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunSubRunEvent") << "RUN_LUMI_EVENT "
                                       << lumi.run() << ", "
                                       << lumi.luminosityBlock() << ", "
                                       << 0;
    }

    if ((index_ + 2U) < expectedRunLumisEvents_.size()) {
      assert(expectedRunLumisEvents_[index_++] == lumi.run());
      assert(expectedRunLumisEvents_[index_++] == lumi.luminosityBlock());
      assert(expectedRunLumisEvents_[index_++] == 0);
    }
  }
}

using edmtest::RunSubRunEventAnalyzer;
DEFINE_FWK_MODULE(RunSubRunEventAnalyzer);
