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

  class ToyProductFilterGlobal : public global::EDFilter {

  public:
    explicit ToyProductFilterGlobal(fhicl::ParameterSet const& pset);

  private:
    bool filter(Event& e) override;

  private:
    string inputLabel_{};
  };

  ToyProductFilterGlobal::ToyProductFilterGlobal(
    fhicl::ParameterSet const& pset)
    : global::EDFilter(), inputLabel_(pset.get<std::string>("inputLabel"))
  {}

  bool
  ToyProductFilterGlobal::filter(Event& /*e*/)
  {
    // e.getValidHandle<StringProduct>(inputLabel_);
    double val = 0.0;
    use_cpu_time(val);
    // cout << val << endl;
    return 1;
  }

} // namespace arttest

DEFINE_ART_GLOBAL_MODULE(arttest::ToyProductFilterGlobal)
