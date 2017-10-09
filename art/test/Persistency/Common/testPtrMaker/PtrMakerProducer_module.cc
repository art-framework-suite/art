////////////////////////////////////////////////////////////////////////
// Class:       PtrMakerProducer
// Plugin Type: producer (art v2_05_00)
// File:        PtrMakerProducer_module.cc
//
// Generated at Tue Nov 22 22:44:53 2016 by Saba Sehrish using cetskelgen
// from cetlib version v1_21_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Persistency/Common/PtrMaker.h"

#include <iostream>
#include <memory>
#include <vector>

namespace arttest {
  class PtrMakerProducer;
}

using arttest::PtrMakerProducer;

class arttest::PtrMakerProducer : public art::EDProducer {
public:
  typedef std::vector<int> intvector_t;
  typedef art::PtrVector<int> intPtrvector_t;

  explicit PtrMakerProducer(fhicl::ParameterSet const& p);

  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  PtrMakerProducer(PtrMakerProducer const&) = delete;
  PtrMakerProducer(PtrMakerProducer&&) = delete;
  PtrMakerProducer& operator=(PtrMakerProducer const&) = delete;
  PtrMakerProducer& operator=(PtrMakerProducer&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

private:
  // Declare member data here.
  std::string fInputLabel;
};

PtrMakerProducer::PtrMakerProducer(fhicl::ParameterSet const& p)
  : fInputLabel(p.get<std::string>("input_label"))
{
  produces<intPtrvector_t>();
}

void
PtrMakerProducer::produce(art::Event& e)
{
  std::cerr << "PtrMakerProducer::produce is running!\n";
  art::Handle<std::vector<int>> h;
  e.getByLabel(fInputLabel, h);
  art::PtrMaker<int> make_intptr(e, h.id());
  auto intptrs = std::make_unique<intPtrvector_t>();
  for (size_t i = 0; i < h->size(); ++i) {
    auto p = make_intptr(i);
    intptrs->push_back(p);
  }
  e.put(std::move(intptrs));
}

DEFINE_ART_MODULE(PtrMakerProducer)
