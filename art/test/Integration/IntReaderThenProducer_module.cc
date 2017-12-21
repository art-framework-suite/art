////////////////////////////////////////////////////////////////////////
// Class:       IntReaderThenProducer
// Plugin Type: producer (art v2_09_03)
// File:        IntReaderThenProducer_module.cc
//
// Generated at Fri Dec 15 11:27:09 2017 by Kyle Knoepfel using cetskelgen
// from cetlib version v3_01_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/types/Atom.h"

#include <memory>

namespace art {
  namespace test {
    class IntReaderThenProducer;
  }
}

class art::test::IntReaderThenProducer : public EDProducer {
public:
  struct Config {
    fhicl::Atom<std::string> inputTag{fhicl::Name{"inputTag"}};
    fhicl::Atom<int> deltaValue{fhicl::Name{"deltaValue"}};
  };
  using Parameters = Table<Config>;
  explicit IntReaderThenProducer(Parameters const& p);

  // Plugins should not be copied or assigned.
  IntReaderThenProducer(IntReaderThenProducer const&) = delete;
  IntReaderThenProducer(IntReaderThenProducer&&) = delete;
  IntReaderThenProducer& operator=(IntReaderThenProducer const&) = delete;
  IntReaderThenProducer& operator=(IntReaderThenProducer&&) = delete;

  // Required functions.
  void produce(Event& e) override;

private:
  ProductToken<arttest::IntProduct> const token_;
  int const deltaValue_;
};

art::test::IntReaderThenProducer::IntReaderThenProducer(Parameters const& p)
  : token_{consumes<arttest::IntProduct>(p().inputTag())}
  , deltaValue_{p().deltaValue()}
{
  produces<arttest::IntProduct>();
}

void
art::test::IntReaderThenProducer::produce(Event& e)
{
  // getValidHandle adds parent for the about-to-be-created IntProduct.
  auto const ivalue = e.getValidHandle(token_)->value;
  e.put(std::make_unique<arttest::IntProduct>(ivalue + deltaValue_));
}

DEFINE_ART_MODULE(art::test::IntReaderThenProducer)
