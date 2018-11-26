#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "fhiclcpp/types/Atom.h"

#include <cassert>

using namespace fhicl;

namespace arttest {
  class DerivedPtrVectorAnalyzer : public art::EDAnalyzer {
  public:
    struct Config {
      Atom<art::InputTag> input_tag{Name{"input_tag"}};
      Atom<bool> require_presence{Name{"require_presence"}};
    };
    using Parameters = Table<Config>;
    explicit DerivedPtrVectorAnalyzer(Parameters const&);

  private:
    void analyze(art::Event const& e) override;

    using product_t = art::PtrVector<arttest::SimpleDerived>;
    art::ProductToken<product_t> const productToken_;
    bool const requirePresence_;
  };

  DerivedPtrVectorAnalyzer::DerivedPtrVectorAnalyzer(Parameters const& p)
    : EDAnalyzer{p}
    , productToken_{consumes<product_t>(p().input_tag())}
    , requirePresence_{p().require_presence()}
  {}

  void
  DerivedPtrVectorAnalyzer::analyze(art::Event const& e)
  {
    art::Handle<product_t> h;
    bool const success = e.getByToken(productToken_, h);
    if (requirePresence_) {
      assert(success);
    } else {
      assert(!success);
    }
  }
}

DEFINE_ART_MODULE(arttest::DerivedPtrVectorAnalyzer)
