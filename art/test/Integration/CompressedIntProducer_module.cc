//--------------------------------------------------------------------
//
// Produces a CompressedIntProduct instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/Integration/RunTimeProduces.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "fhiclcpp/ParameterSet.h"

#include "art/test/TestObjects/ToyProducts.h"

#include <iostream>
#include <memory>

namespace arttest {
  class CompressedIntProducer;
}

using arttest::CompressedIntProducer;

class arttest::CompressedIntProducer : public art::EDProducer {
public:
  explicit CompressedIntProducer(fhicl::ParameterSet const& p)
    : EDProducer{p}
    , value_(p.get<int>("ivalue"))
    ,
    // enums don't usually have a conversion from string
    branchType_(
      art::BranchType(p.get<unsigned long>("branchType", art::InEvent)))
  {
    art::test::run_time_produces<CompressedIntProduct>(this, branchType_);
  }

private:
  void produce(art::Event& e) override;
  void endSubRun(art::SubRun& sr) override;
  void endRun(art::Run& r) override;

  int const value_;
  art::BranchType const branchType_;

}; // CompressedIntProducer

void
CompressedIntProducer::produce(art::Event& e)
{
  if (branchType_ == art::InEvent)
    e.put(std::make_unique<CompressedIntProduct>(value_));
}

void
CompressedIntProducer::endSubRun(art::SubRun& sr)
{
  if (branchType_ == art::InSubRun)
    sr.put(std::make_unique<CompressedIntProduct>(value_),
           art::subRunFragment());
}

void
CompressedIntProducer::endRun(art::Run& r)
{
  if (branchType_ == art::InRun)
    r.put(std::make_unique<CompressedIntProduct>(value_), art::runFragment());
}

DEFINE_ART_MODULE(CompressedIntProducer)
