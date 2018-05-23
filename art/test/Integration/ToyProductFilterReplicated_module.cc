// vim: set sw=2 expandtab :
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/ReplicatedFilter.h"
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

  class ToyProductFilterReplicated : public ReplicatedFilter {
  public:
    struct Config {
    };
    using Parameters = Table<Config>;
    explicit ToyProductFilterReplicated(Parameters const& p, art::ScheduleID);

  private:
    bool filter(Event& e, Services const&) override;
  };

  ToyProductFilterReplicated::ToyProductFilterReplicated(
    Parameters const& p,
    art::ScheduleID const sid)
    : ReplicatedFilter{p, sid}
  {}

  bool
  ToyProductFilterReplicated::filter(Event& /*e*/, Services const&)
  {
    double val = 0.0;
    use_cpu_time(val);
    return true;
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::ToyProductFilterReplicated)
