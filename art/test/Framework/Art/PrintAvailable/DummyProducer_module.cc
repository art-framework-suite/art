////////////////////////////////////////////////////////////////////////
// Class:       DummyProducer
// Plugin Type: producer (art v2_06_03)
// File:        DummyProducer_module.cc
//
// Generated at Fri May 19 09:54:57 2017 by Kyle Knoepfel using cetskelgen
// from cetlib version v2_03_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"

namespace art {
  class Event;
  namespace test {
    class DummyProducer;
  }
} // namespace art

class art::test::DummyProducer : public EDProducer {
public:
  struct Config {
  };
  using Parameters = EDProducer::Table<Config>;
  explicit DummyProducer(Parameters const&) {}

private:
  void
  produce(Event&) override
  {}
};

DEFINE_ART_MODULE(art::test::DummyProducer)
