////////////////////////////////////////////////////////////////////////
// Class:       DummyFilter
// Plugin Type: filter (art v2_06_03)
// File:        DummyFilter_module.cc
//
// Generated at Fri May 19 09:55:59 2017 by Kyle Knoepfel using cetskelgen
// from cetlib version v2_03_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"

namespace art {
  class Event;
  namespace test {
    class DummyFilter;
  }
} // namespace art

class art::test::DummyFilter : public EDFilter {
public:
  struct Config {
  };
  using Parameters = EDFilter::Table<Config>;
  explicit DummyFilter(Parameters const& ps) : EDFilter{ps} {}

private:
  bool
  filter(Event&) override
  {
    return true;
  }
};

DEFINE_ART_MODULE(art::test::DummyFilter)
