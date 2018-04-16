// vim: set sw=2 expandtab :

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/View.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/types/Atom.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace art;
using namespace fhicl;

namespace arttest {

  class IntVectorAnalyzer : public EDAnalyzer {
    // Config
  public:
    struct Config {
      Atom<string> input_label{Name("input_label")};
      Atom<size_t> nvalues{Name("nvalues")};
    };
    using Parameters = Table<Config>;
    // Special Member Functions
  public:
    explicit IntVectorAnalyzer(Parameters const& p);
    // API
  public:
    void analyze(Event const&) override;
    // Data Members
  private:
    string moduleLabel_;
    size_t nvalues_;
    ViewToken<int> viewToken_;
  };

  IntVectorAnalyzer::IntVectorAnalyzer(Parameters const& p)
    : EDAnalyzer{p}
    , moduleLabel_{p().input_label()}
    , nvalues_{p().nvalues()}
    , viewToken_{consumesView<int>(moduleLabel_)}
  {}

  void
  IntVectorAnalyzer::analyze(Event const& e)
  {
    {
      vector<int const*> ptrs;
      size_t sz = e.getView(moduleLabel_, ptrs);
      if (sz != nvalues_) {
        cerr << "SizeMismatch expected a view of size " << nvalues_
             << " but the obtained size is " << sz << '\n';
        throw cet::exception("SizeMismatch")
          << "Expected a view of size " << nvalues_
          << " but the obtained size is " << sz << '\n';
      }
      for (size_t k = 0; k != sz; ++k) {
        if (*ptrs[k] != (int)(e.id().event() + k)) {
          cerr << "ValueMismatch at position " << k << " expected value "
               << e.id().event() + k << " but obtained " << *ptrs[k] << '\n';
          throw cet::exception("ValueMismatch")
            << "At position " << k << " expected value " << e.id().event() + k
            << " but obtained " << *ptrs[k] << '\n';
        }
      }
    }
    {
      View<int> vw;
      e.getView(viewToken_, vw);
      assert(vw.isValid());
      // Test that the range-for loop compiles.
      for (auto it : vw) {
        assert(*it >= 0);
      }
    }
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::IntVectorAnalyzer)
