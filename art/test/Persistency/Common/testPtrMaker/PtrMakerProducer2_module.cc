////////////////////////////////////////////////////////////////////////
// Class:       PtrMakerProducer2
// Plugin Type: producer (art v2_05_00)
// File:        PtrMakerProducer2_module.cc
//
// Generated at Wed Nov 23 22:59:41 2016 by Saba Sehrish using cetskelgen
// from cetlib version v1_21_00.
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
#include "art/Persistency/Common/PtrMaker.h"
#include <memory>

class PtrMakerProducer2;


class PtrMakerProducer2 : public art::EDProducer {
public:
  typedef  std::vector<int>     intvector_t;
  typedef  art::PtrVector<int>  intPtrvector_t;
  explicit PtrMakerProducer2(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  PtrMakerProducer2(PtrMakerProducer2 const &) = delete;
  PtrMakerProducer2(PtrMakerProducer2 &&) = delete;
  PtrMakerProducer2 & operator = (PtrMakerProducer2 const &) = delete;
  PtrMakerProducer2 & operator = (PtrMakerProducer2 &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

private:

  int nvalues;
};


PtrMakerProducer2::PtrMakerProducer2(fhicl::ParameterSet const & p)
: nvalues( p.get<int>("nvalues") )
  {
    produces<intvector_t>();
    produces<intPtrvector_t>();
  }

void PtrMakerProducer2::produce(art::Event & e)
{
  std::cerr << "PtrMakerProducer 2 is running!\n";
  int value_ = e.id().event();
  std::unique_ptr<intvector_t> intvector(new intvector_t);
  auto intptrs = std::make_unique<intPtrvector_t>();
  art::PtrMaker<int> make_intptr(e, *this);

  for( int i = 0; i != nvalues; ++i ) {
    intvector->push_back(value_ + i);
    auto p = make_intptr(i);
    intptrs->push_back(p);
  }
  e.put(std::move(intvector));
  e.put(std::move(intptrs));
}

DEFINE_ART_MODULE(PtrMakerProducer2)
