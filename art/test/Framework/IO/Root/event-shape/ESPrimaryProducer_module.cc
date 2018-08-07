////////////////////////////////////////////////////////////////////////
// Class:       ESPrimaryProducer
// Module Type: producer
// File:        ESPrimaryProducer_module.cc
//
// Generated at Mon Feb  3 11:00:13 2014 by Christopher Green using artmod
// from cetpkgsupport v1_05_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace arttest {
  class ESPrimaryProducer;
}

class arttest::ESPrimaryProducer : public art::EDProducer {
public:
  explicit ESPrimaryProducer(fhicl::ParameterSet const&);

private:
  void produce(art::Event& e) override;
};

arttest::ESPrimaryProducer::ESPrimaryProducer(fhicl::ParameterSet const& ps)
  : EDProducer{ps}
{
  produces<arttest::VSimpleProduct>();
}

void
arttest::ESPrimaryProducer::produce(art::Event& e)
{
  constexpr size_t vspSize{5};
  constexpr double vspVal{1.5};
  auto vsp = std::make_unique<VSimpleProduct>(vspSize);

  size_t count{};
  for (auto& s : *vsp) {
    s.key = count;
    s.value = count * vspVal;
    ++count;
  }
  e.put(std::move(vsp));
}

DEFINE_ART_MODULE(arttest::ESPrimaryProducer)
