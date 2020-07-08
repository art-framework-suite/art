////////////////////////////////////////////////////////////////////////
// Class:       PMTestProducer
// Module Type: producer
// File:        PMTestProducer_module.cc
//
// Generated at Mon Jun 17 16:44:18 2013 by Christopher Green using artmod
// from cetpkgsupport v1_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace arttest {
  class PMTestProducer;
}

class arttest::PMTestProducer : public art::EDProducer {
public:
  explicit PMTestProducer(fhicl::ParameterSet const& p);

private:
  void produce(art::Event&) override;
};

arttest::PMTestProducer::PMTestProducer(fhicl::ParameterSet const& p)
  : EDProducer{p}
{}

void
arttest::PMTestProducer::produce(art::Event&)
{}

DEFINE_ART_MODULE(arttest::PMTestProducer)
