////////////////////////////////////////////////////////////////////////
// Class:       ConstAssnsIterAnalyzer
// File:        ConstAssnsIterAnalyzer_module.cc
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/AssnsIter.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

class ConstAssnsIterAnalyzer : public art::EDAnalyzer {
public:
  using intvec_t = std::vector<int>;
  using strvec_t = std::vector<std::string>;
  using floatvec_t = std::vector<float>;

  explicit ConstAssnsIterAnalyzer(fhicl::ParameterSet const& p);

  // Plugins should not be copied or assigned.
  ConstAssnsIterAnalyzer(ConstAssnsIterAnalyzer const&) = delete;
  ConstAssnsIterAnalyzer(ConstAssnsIterAnalyzer&&) = delete;
  ConstAssnsIterAnalyzer& operator=(ConstAssnsIterAnalyzer const&) = delete;
  ConstAssnsIterAnalyzer& operator=(ConstAssnsIterAnalyzer&&) = delete;

private:
  // Required functions.
  void analyze(art::Event const& e) override;

  art::InputTag const fInputLabel;
};

ConstAssnsIterAnalyzer::ConstAssnsIterAnalyzer(fhicl::ParameterSet const& p)
  : EDAnalyzer{p}, fInputLabel{p.get<art::InputTag>("input_label")}
{}

void
ConstAssnsIterAnalyzer::analyze(art::Event const& e)
{
  using assns_t = art::Assns<int, float, std::string>;

  // vectors to verify values
  auto const vi = intvec_t{1, 1, 2, 2, 3, 3};
  auto const vs = strvec_t{"one", "one-a", "two", "two-a", "three", "three-a"};
  auto const vf = floatvec_t{1.0, 1.1, 2.0, 2.1, 3.0, 3.1};

  assns_t const& assns{*e.getValidHandle<assns_t>(fInputLabel)};

  // iterator increment and dereference test
  auto my_begin = assns.begin();
  auto my_end = assns.end();
  assert(my_begin != my_end);
  int k = 0;
  for (auto p = my_begin; p != my_end; ++p) {
    assert(*((*p).data) == vs[k]);
    assert(*((*p).second.get()) == vf[k]);
    assert(*((*p).first.get()) == vi[k]);
    ++k;
  }
}

DEFINE_ART_MODULE(ConstAssnsIterAnalyzer)
