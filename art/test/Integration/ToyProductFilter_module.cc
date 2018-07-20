// vim: set sw=2 expandtab :
#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/ParameterSet.h"

#include <cmath>
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

  class ToyProductFilter : public EDFilter {
  public:
    struct Config {
    };
    using Parameters = Table<Config>;
    explicit ToyProductFilter(Parameters const& pset);

  private:
    bool filter(Event& e) override;
  };

  ToyProductFilter::ToyProductFilter(Parameters const& pset) : EDFilter{pset} {}

  bool
  ToyProductFilter::filter(Event& /*e*/)
  {
    double val{};
    use_cpu_time(val);
    return true;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ToyProductFilter)
