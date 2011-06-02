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
#include "art/Persistency/Common/OrphanHandle.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"

#include "cetlib/map_vector.h"

#include <string>

namespace arttest {
  class PtrmvProducer;
}

namespace {
  typedef cet::map_vector<std::string> mv_t;
  typedef typename mv_t::value_type mvp_t;
}

class arttest::PtrmvProducer : public art::EDProducer {
public:
  explicit PtrmvProducer(fhicl::ParameterSet const &p);
  virtual ~PtrmvProducer();

  virtual void produce(art::Event &e);
};

arttest::PtrmvProducer::PtrmvProducer(fhicl::ParameterSet const &p)
{
  produces<mv_t>();
  produces<art::Ptr<std::string> >();
  produces<art::PtrVector<std::string> >();
  produces<art::Ptr<mvp_t> >();
  produces<art::PtrVector<mvp_t> >();
}

arttest::PtrmvProducer::~PtrmvProducer() {
}

void arttest::PtrmvProducer::produce(art::Event &e) {
  std::auto_ptr<mv_t>
    mv(new mv_t);
  mv->reserve(4);
  (*mv)[cet::map_vector_key(0)] = "ONE";
  (*mv)[cet::map_vector_key(3)] = "TWO";
  (*mv)[cet::map_vector_key(5)] = "THREE";
  (*mv)[cet::map_vector_key(7)] = "FOUR";

  art::OrphanHandle<mv_t>
    mvHandle(e.put(mv));

  e.put(std::auto_ptr<art::Ptr<std::string> >(new art::Ptr<std::string>(mvHandle, 3)));

  std::auto_ptr<art::PtrVector<std::string> >
    pv(new art::PtrVector<std::string>());

  pv->reserve(4);
  pv->push_back(art::Ptr<std::string>(mvHandle, 5));
  pv->push_back(art::Ptr<std::string>(mvHandle, 0));
  pv->push_back(art::Ptr<std::string>(mvHandle, 7));
  pv->push_back(art::Ptr<std::string>(mvHandle, 3));

  e.put(pv);

  e.put(std::auto_ptr<art::Ptr<mvp_t> >(new art::Ptr<mvp_t>(mvHandle, 3)));

  std::auto_ptr<art::PtrVector<mvp_t> >
    pvp(new art::PtrVector<mvp_t>());

  pvp->reserve(4);
  pvp->push_back(art::Ptr<mvp_t>(mvHandle, 5));
  pvp->push_back(art::Ptr<mvp_t>(mvHandle, 0));
  pvp->push_back(art::Ptr<mvp_t>(mvHandle, 7));
  pvp->push_back(art::Ptr<mvp_t>(mvHandle, 3));

  e.put(pvp);
}

DEFINE_ART_MODULE(arttest::PtrmvProducer);
