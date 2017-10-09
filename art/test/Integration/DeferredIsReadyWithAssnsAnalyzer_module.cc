////////////////////////////////////////////////////////////////////////
// Class:       DeferredIsReadyWithAssnsAnalyzer
// Module Type: analyzer
// File:        DeferredIsReadyWithAssnsAnalyzer_module.cc
//
// Generated at Wed Nov  6 15:11:26 2013 by Christopher Green using artmod
// from cetpkgsupport v1_04_02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/AssnTestData.h"
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class DeferredIsReadyWithAssnsAnalyzer;
}

class arttest::DeferredIsReadyWithAssnsAnalyzer : public art::EDAnalyzer {
public:
  explicit DeferredIsReadyWithAssnsAnalyzer(fhicl::ParameterSet const& p);

  void analyze(art::Event const& e) override;

private:
  std::string inputLabel_;
};

arttest::DeferredIsReadyWithAssnsAnalyzer::DeferredIsReadyWithAssnsAnalyzer(
  fhicl::ParameterSet const& p)
  : EDAnalyzer(p), inputLabel_(p.get<std::string>("input_label"))
{}

void
arttest::DeferredIsReadyWithAssnsAnalyzer::analyze(art::Event const& e)
{
  auto h = e.getValidHandle<std::vector<size_t>>(inputLabel_);
  art::PtrVector<size_t> pv;
  pv.reserve(h->size());
  for (size_t i = 0, e = h->size(); i != e; ++i) {
    pv.push_back(art::Ptr<size_t>(h, i));
  }
  art::FindMany<std::string, arttest::AssnTestData> fmp(pv, e, inputLabel_);
  for (size_t i = 0, sz = fmp.size(); i != sz; ++i) {
    assert(fmp.at(i).size() > 0);
    assert(*fmp.at(i)[0] == std::string(1, i + 'a'));
  }
}

DEFINE_ART_MODULE(arttest::DeferredIsReadyWithAssnsAnalyzer)
