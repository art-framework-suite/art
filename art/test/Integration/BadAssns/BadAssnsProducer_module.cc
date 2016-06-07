////////////////////////////////////////////////////////////////////////
// Class:       BadAssnsProducer
// Plugin Type: producer (art v1_19_00_rc3)
// File:        BadAssnsProducer_module.cc
//
// Generated at Thu Apr 14 08:54:19 2016 by Christopher Green using cetskelgen
// from cetlib version v1_17_04.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/test/TestObjects/ToyProducts.h"

#include <memory>

namespace arttest {
  class BadAssnsProducer;
}


class arttest::BadAssnsProducer : public art::EDProducer {
public:
  explicit BadAssnsProducer(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  BadAssnsProducer(BadAssnsProducer const &) = delete;
  BadAssnsProducer(BadAssnsProducer &&) = delete;
  BadAssnsProducer & operator = (BadAssnsProducer const &) = delete;
  BadAssnsProducer & operator = (BadAssnsProducer &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

private:

};


arttest::BadAssnsProducer::BadAssnsProducer(fhicl::ParameterSet const &)
{
  produces<art::Assns<StringProduct, DummyProduct>>();
}

void arttest::BadAssnsProducer::produce(art::Event & e)
{
  e.put(std::make_unique<art::Assns<StringProduct, DummyProduct> >());
}

DEFINE_ART_MODULE(arttest::BadAssnsProducer)
