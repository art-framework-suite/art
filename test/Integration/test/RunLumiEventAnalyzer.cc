
#include "FWCore/Integration/test/RunLumiEventAnalyzer.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/LuminosityBlock.h"
#include "art/Framework/Core/Run.h"
#include "art/ParameterSet/ParameterSet.h"

#include <cassert>
#include <iostream>

namespace edmtest {

  RunLumiEventAnalyzer::RunLumiEventAnalyzer(edm::ParameterSet const& pset) :
    expectedRunLumisEvents_(pset.getUntrackedParameter<std::vector<unsigned int> >("expectedRunLumiEvents", std::vector<unsigned int>())),
    index_(0),
    verbose_(pset.getUntrackedParameter<bool>("verbose", false)) {
  }

  void RunLumiEventAnalyzer::analyze(edm::Event const& event, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunLumiEvent") << "RUN_LUMI_EVENT "
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

  void RunLumiEventAnalyzer::beginRun(edm::Run const& run, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunLumiEvent") << "RUN_LUMI_EVENT "
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

  void RunLumiEventAnalyzer::endRun(edm::Run const& run, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunLumiEvent") << "RUN_LUMI_EVENT "
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

  void RunLumiEventAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const& lumi, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunLumiEvent") << "RUN_LUMI_EVENT "
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

  void RunLumiEventAnalyzer::endLuminosityBlock(edm::LuminosityBlock const& lumi, edm::EventSetup const& es) {

    if (verbose_) {
      edm::LogAbsolute("RunLumiEvent") << "RUN_LUMI_EVENT "
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

using edmtest::RunLumiEventAnalyzer;
DEFINE_FWK_MODULE(RunLumiEventAnalyzer);
