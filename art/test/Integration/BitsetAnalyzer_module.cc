// ======================================================================
// Tests an std::bitset<4> instance.
// ======================================================================

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

#include <bitset>

namespace {
  constexpr std::size_t sz{4u};
}

namespace art::test {

  class BitsetAnalyzer : public art::SharedAnalyzer {
  public:
    struct Config {
      fhicl::Atom<std::string> module_label{fhicl::Name{"moduleLabel"}};
    };
    using Parameters = Table<Config>;

    explicit BitsetAnalyzer(Parameters const& p, art::ProcessingFrame const&);

  private:
    void analyze(art::Event const& e, art::ProcessingFrame const&) override;

    art::InputTag const tag_;

  }; // BitsetAnalyzer

  BitsetAnalyzer::BitsetAnalyzer(Parameters const& p,
                                 art::ProcessingFrame const&)
    : SharedAnalyzer{p}, tag_{p().module_label()}
  {
    async<InEvent>();
  }

  void
  BitsetAnalyzer::analyze(art::Event const& e, art::ProcessingFrame const&)
  {
    auto const set = *e.getValidHandle<std::bitset<sz>>(tag_);
    assert(set == std::bitset<sz>{0b1011});
  }

} // art::test

DEFINE_ART_MODULE(art::test::BitsetAnalyzer)
