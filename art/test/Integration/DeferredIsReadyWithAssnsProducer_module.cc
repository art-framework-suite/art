////////////////////////////////////////////////////////////////////////
// Class:       DeferredIsReadyWithAssnsProducer
// Module Type: producer
// File:        DeferredIsReadyWithAssnsProducer_module.cc
//
// Generated at Wed Nov  6 17:21:54 2013 by Christopher Green using artmod
// from cetpkgsupport v1_04_02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/TestObjects/AssnTestData.h"

#include <memory>

namespace arttest {
  class DeferredIsReadyWithAssnsProducer;
}

class arttest::DeferredIsReadyWithAssnsProducer : public art::EDProducer {
public:
  explicit DeferredIsReadyWithAssnsProducer(fhicl::ParameterSet const & p);

  void produce(art::Event & e) override;

private:

};


arttest::DeferredIsReadyWithAssnsProducer::DeferredIsReadyWithAssnsProducer(fhicl::ParameterSet const &)
{
  produces<std::vector<std::string> >();
  produces<std::vector<size_t> >();
  produces<art::Assns<std::string, size_t, arttest::AssnTestData> >();
}

void arttest::DeferredIsReadyWithAssnsProducer::produce(art::Event & e)
{
  std::unique_ptr<std::vector<std::string> > vs { new std::vector<std::string> { "f", "e", "d", "c", "b", "a" } };
  std::unique_ptr<std::vector<size_t> > vi { new std::vector<size_t> { 0, 1, 2, 3, 4, 5 } };

  auto sz = vs->size();

  auto vspid (e.put(std::move(vs)));
  auto vipid (e.put(std::move(vi)));

  std::unique_ptr<art::Assns<std::string, size_t, arttest::AssnTestData> >
    asid { new art::Assns<std::string, size_t, arttest::AssnTestData> };
  for (size_t i = 0; i != sz; ++i) {
    asid->addSingle({ vspid, sz - i - 1, e.productGetter(vspid) },
                   { vipid, i, e.productGetter(vipid) },
                   { i, i, "Ethel" });
  }

  e.put(std::move(asid));
}

DEFINE_ART_MODULE(arttest::DeferredIsReadyWithAssnsProducer)
