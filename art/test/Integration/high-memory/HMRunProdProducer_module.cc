////////////////////////////////////////////////////////////////////////
// Class:       HMRunProdProducer
// Module Type: producer
// File:        HMRunProdProducer_module.cc
//
// Generated at Tue Apr 15 13:28:33 2014 by Christopher Green using artmod
// from cetpkgsupport v1_05_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/Integration/high-memory/HMLargeData.h"

#include <memory>
#include <string>
#include <vector>

namespace arttest {
  class HMRunProdProducer;
}

class arttest::HMRunProdProducer : public art::EDProducer {
public:
  explicit HMRunProdProducer(fhicl::ParameterSet const & p);
  void produce(art::Event &) override { };
  void endSubRun(art::SubRun & sr) override;
  void endRun(art::Run & r) override;

private:
  std::string inputLabel_;
  std::vector<std::unique_ptr<HMLargeData> > data_;
};

arttest::HMRunProdProducer::HMRunProdProducer(fhicl::ParameterSet const & p)
:
  inputLabel_(p.get<std::string>("inputLabel")),
  data_(N_BLOCKS)
{
  for (unsigned short i = 0; i < N_BLOCKS; ++i) {
    produces<HMLargeData, art::InRun>(std::string("block") + std::to_string(i));
  }
}

void arttest::HMRunProdProducer::endSubRun(art::SubRun & sr)
{
  for (unsigned short i = 0; i < N_BLOCKS; ++i) {
    art::Handle<HMLargeData> h;
    sr.getByLabel<HMLargeData>(inputLabel_,
                               std::string("block") + std::to_string(i), h);
    if (data_[i]) {
      *data_[i] += *h;
    } else {
      data_[i] = std::make_unique<HMLargeData>(*h);
    }
    assert(sr.removeCachedProduct(h)); // Save the space.
  }
}

void arttest::HMRunProdProducer::endRun(art::Run & r)
{
  for (unsigned short i = 0; i < N_BLOCKS; ++i) {
    r.put(std::move(data_[i]), std::string("block") + std::to_string(i));
  }
}

DEFINE_ART_MODULE(arttest::HMRunProdProducer)
