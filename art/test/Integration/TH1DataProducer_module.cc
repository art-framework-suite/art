////////////////////////////////////////////////////////////////////////
// Class:       TH1DataProducer
// Module Type: producer
// File:        TH1DataProducer_module.cc
//
// Generated at Mon Aug 19 18:01:04 2013 by Chris Green using artmod
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
#include "art/test/TestObjects/TH1Data.h"

#include <memory>

namespace arttest {
  class TH1DataProducer;
}

class arttest::TH1DataProducer : public art::EDProducer {
public:
  explicit TH1DataProducer(fhicl::ParameterSet const & p);

  void produce(art::Event & e) override;

  void beginRun(art::Run & r) override;
  void endRun(art::Run & r) override;

private:

  // Declare member data here.
  std::unique_ptr<arttest::TH1Data> data_;
};


arttest::TH1DataProducer::TH1DataProducer(fhicl::ParameterSet const &)
 :
  data_(new arttest::TH1Data)
{
  data_->data = TH1D("name", "title", 101, -.5, 100.5);
  produces<arttest::TH1Data, art::InRun>();
}

void arttest::TH1DataProducer::produce(art::Event & e)
{
  data_->data.Fill(e.event() % 100, 1.);
}

void arttest::TH1DataProducer::beginRun(art::Run &)
{
  // Implementation of optional member function here.
}

void arttest::TH1DataProducer::endRun(art::Run & r)
{
  r.put(std::move(data_), art::runFragment());
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(arttest::TH1DataProducer)
