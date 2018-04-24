// vim: set sw=2 expandtab :
#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
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
    explicit ToyProductFilterAsync(fhicl::ParameterSet const& pset);

  private:
    bool filter(Event& e) override;

  private:
    string inputLabel_{};
  };

  ToyProductFilterAsync::ToyProductFilterAsync(fhicl::ParameterSet const& pset)
    : inputLabel_(pset.get<std::string>("inputLabel"))
  {
    // FIXME: Will end up calling async<Event>();
  }

  bool
  ToyProductFilterAsync::filter(Event& /*e*/)
  {
    double val = 0.0;
    use_cpu_time(val);
    return true;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ToyProductFilterAsync)
