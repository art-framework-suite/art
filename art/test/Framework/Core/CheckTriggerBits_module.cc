#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/SharedAnalyzer.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Utilities/InputTag.h"

namespace {
  class CheckTriggerBits : public art::SharedAnalyzer {
  public:
    struct Config {
      fhicl::Sequence<std::string> paths{fhicl::Name{"paths"}};
    };
    using Parameters = Table<Config>;
    explicit CheckTriggerBits(Parameters const& p,
                              art::ProcessingFrame const&);

  private:
    void analyze(art::Event const&,
                 art::ProcessingFrame const&) override;
    std::vector<std::string> const paths_;
  };

  CheckTriggerBits::CheckTriggerBits(Parameters const& p,
                                     art::ProcessingFrame const&)
    : SharedAnalyzer{p}
    , paths_{p().paths()}
  {
    async<art::InEvent>();

    art::ServiceHandle<art::TriggerNamesService const> triggerNames;
    auto const num_paths = triggerNames->size();
    BOOST_TEST(num_paths == size(paths_));

    for (std::size_t i{}; i != num_paths; ++i) {
      BOOST_TEST(triggerNames->getTrigPath(i) == paths_[i]);
    }
  }

  void
  CheckTriggerBits::analyze(art::Event const&,
                            art::ProcessingFrame const&)
  {}
}

DEFINE_ART_MODULE(CheckTriggerBits)
