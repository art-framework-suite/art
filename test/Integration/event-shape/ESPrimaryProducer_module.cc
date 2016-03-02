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
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"

#include <memory>

namespace arttest {
  class ESPrimaryProducer;
}

class arttest::ESPrimaryProducer : public art::EDProducer {
public:
  explicit ESPrimaryProducer(fhicl::ParameterSet const &);

  void produce(art::Event & e) override;

private:

};


arttest::ESPrimaryProducer::ESPrimaryProducer(fhicl::ParameterSet const &)
{
  produces<arttest::VSimpleProduct>();
}

void arttest::ESPrimaryProducer::produce(art::Event & e)
{
  static size_t constexpr vspSize { 5 };
  static double constexpr vspVal = { 1.5 };
  std::unique_ptr<VSimpleProduct> vsp(new VSimpleProduct(vspSize));

  size_t count { 0 };
  for (auto & s : *vsp ) {
    s.key = count;
    s.value = count++ * vspVal;
  }
  e.put(std::move(vsp));
}

DEFINE_ART_MODULE(arttest::ESPrimaryProducer)
