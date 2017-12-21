////////////////////////////////////////////////////////////////////////
// Class:       PtrmvProducer
// Module Type: producer
// File:        PtrmvProducer_module.cc
//
// Generated at Tue May 31 08:00:52 2011 by Chris Green using artmod
// from art v0_07_07.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"

#include "cetlib/map_vector.h"

#include <string>

namespace arttest {
  class PtrmvProducer;
}

namespace {
  using mv_t = cet::map_vector<std::string>;
  using mvp_t = mv_t::value_type;
}

class arttest::PtrmvProducer : public art::EDProducer {
public:
  struct Config {
  };
  using Parameters = Table<Config>;
  explicit PtrmvProducer(Parameters const&);

  void produce(art::Event& e) override;
};

arttest::PtrmvProducer::PtrmvProducer(Parameters const&)
{
  produces<mv_t>();
  produces<art::Ptr<std::string>>();
  produces<art::PtrVector<std::string>>();
  produces<art::Ptr<mvp_t>>();
  produces<art::PtrVector<mvp_t>>();
}

void
arttest::PtrmvProducer::produce(art::Event& e)
{
  auto mv = std::make_unique<mv_t>();
  mv->reserve(4);
  (*mv)[cet::map_vector_key(0)] = "ONE";
  (*mv)[cet::map_vector_key(3)] = "TWO";
  (*mv)[cet::map_vector_key(5)] = "THREE";
  (*mv)[cet::map_vector_key(7)] = "FOUR";

  art::ProductID const mvID{e.put(std::move(mv))};

  e.put(
    std::make_unique<art::Ptr<std::string>>(mvID, 3, e.productGetter(mvID)));

  auto pv = std::make_unique<art::PtrVector<std::string>>();
  pv->reserve(4);
  pv->push_back(art::Ptr<std::string>{mvID, 5, e.productGetter(mvID)});
  pv->push_back(art::Ptr<std::string>{mvID, 0, e.productGetter(mvID)});
  pv->push_back(art::Ptr<std::string>{mvID, 7, e.productGetter(mvID)});
  pv->push_back(art::Ptr<std::string>{mvID, 3, e.productGetter(mvID)});

  e.put(std::move(pv));
  e.put(std::make_unique<art::Ptr<mvp_t>>(mvID, 3, e.productGetter(mvID)));

  auto pvp = std::make_unique<art::PtrVector<mvp_t>>();
  pvp->reserve(4);
  pvp->push_back(art::Ptr<mvp_t>{mvID, 5, e.productGetter(mvID)});
  pvp->push_back(art::Ptr<mvp_t>{mvID, 0, e.productGetter(mvID)});
  pvp->push_back(art::Ptr<mvp_t>{mvID, 7, e.productGetter(mvID)});
  pvp->push_back(art::Ptr<mvp_t>{mvID, 3, e.productGetter(mvID)});

  e.put(std::move(pvp));
}

DEFINE_ART_MODULE(arttest::PtrmvProducer)
