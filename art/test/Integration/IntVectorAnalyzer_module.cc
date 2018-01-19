// ======================================================================
//
// IntVectorAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/View.h"
#include "cetlib/exception.h"
#include "fhiclcpp/types/Atom.h"
#include <iostream>
#include <string>
#include <vector>

namespace {

  using namespace fhicl;
  struct Config {
    Atom<std::string> input_label{Name("input_label")};
    Atom<size_t> nvalues{Name("nvalues")};
  };

} // namespace

namespace arttest {
  class IntVectorAnalyzer;
}

// ----------------------------------------------------------------------

class arttest::IntVectorAnalyzer : public art::EDAnalyzer {
public:
  using intvector_t = std::vector<int>;

  using Parameters = Table<Config>;
  explicit IntVectorAnalyzer(Parameters const& p)
    : art::EDAnalyzer{p}
    , moduleLabel_{p().input_label()}
    , nvalues_{p().nvalues()}
    , viewToken_{consumesView<int>(moduleLabel_)}
  {}

  void
  analyze(art::Event const& e) override
  {
    test_vector(e);
    test_view(e);
  }

  void
  test_view(art::Event const& e)
  {
    art::View<int> vw;
    e.getView(viewToken_, vw);
    assert(vw.isValid());

    // Test that the range-for loop compiles.
    for (auto it : vw) {
      assert(*it >= 0);
    }
  }

  void
  test_vector(art::Event const& e)
  {
    std::vector<int const*> ptrs;
    size_t sz = e.getView(moduleLabel_, ptrs);

    if (sz != nvalues_) {
      std::cerr << "SizeMismatch expected a view of size " << nvalues_
                << " but the obtained size is " << sz << '\n';
      throw cet::exception("SizeMismatch")
        << "Expected a view of size " << nvalues_
        << " but the obtained size is " << sz << '\n';
    }

    art::EventNumber_t value_ = e.id().event();
    for (size_t k = 0; k != sz; ++k) {
      if (*ptrs[k] != (int)(value_ + k)) {
        std::cerr << "ValueMismatch at position " << k << " expected value "
                  << value_ + k << " but obtained " << *ptrs[k] << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k << " expected value " << value_ + k
          << " but obtained " << *ptrs[k] << '\n';
      }
    }
  } // analyze()

private:
  std::string moduleLabel_;
  size_t nvalues_;
  art::ViewToken<int> viewToken_;

}; // IntVectorAnalyzer

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(arttest::IntVectorAnalyzer)

// ======================================================================
