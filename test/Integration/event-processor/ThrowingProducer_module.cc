#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/types/Atom.h"

namespace {

  struct Config {
    fhicl::Atom<bool> doThrow { fhicl::Name("throw") };
  };

}

namespace arttest {

  class ThrowingProducer : public art::EDProducer {
    bool doThrow_{false};
  public:

    using Parameters = art::EDProducer::Table<Config>;

    ThrowingProducer(Parameters const& c)
      : doThrow_( c().doThrow() )
    {
      if ( doThrow_ )
        throw art::Exception(art::errors::Configuration, "Throwing producer ctor");
    }

    void produce(art::Event&) override {}
    
  };

}

DEFINE_ART_MODULE(arttest::ThrowingProducer)
