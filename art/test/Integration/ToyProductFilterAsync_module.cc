// vim: set sw=2 expandtab :
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedFilter.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace art;
using namespace std;

namespace {

  double
  f(int val)
  {
    return sqrt(val);
  }

  void
  use_cpu_time(double& val)
  {
    for (int i = 0; i < 100'000'000; ++i) {
      val = f(i);
    }
  }

} // unnamed namespace

namespace arttest {

  class ToyProductFilterAsync : public SharedFilter {
  public:
    struct Config {
      fhicl::Atom<std::string> inputLabel{fhicl::Name{"inputLabel"}};
    };
    using Parameters = Table<Config>;
    explicit ToyProductFilterAsync(Parameters const& p, ProcessingFrame const&);

  private:
    bool filter(Event& e, ProcessingFrame const&) override;

    string const inputLabel_{};
  };

  ToyProductFilterAsync::ToyProductFilterAsync(Parameters const& pset,
                                               ProcessingFrame const&)
    : SharedFilter{pset}, inputLabel_{pset().inputLabel()}
  {
    async<InEvent>();
  }

  bool
  ToyProductFilterAsync::filter(Event&, ProcessingFrame const&)
  {
    double val = 0.0;
    use_cpu_time(val);
    return true;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ToyProductFilterAsync)
