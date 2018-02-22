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
    : value_(p.get<int>("ivalue"))
    ,
    // enums don't usually have a conversion from string
    branchType_(
      art::BranchType(p.get<unsigned long>("branchType", art::InEvent)))
  {
    art::test::run_time_produces<CompressedIntProduct>(this, branchType_);
  }

  explicit CompressedIntProducer(int i) : value_(i)
  {
    produces<CompressedIntProduct>();
  }

  void produce(art::Event& e) override;
  void endSubRun(art::SubRun& sr) override;
  void endRun(art::Run& r) override;

private:
  int value_;
  art::BranchType branchType_;

}; // CompressedIntProducer

void
CompressedIntProducer::produce(art::Event& e)
{
  std::cerr << "Holy cow, CompressedIntProducer::produce is running!\n";
  if (branchType_ == art::InEvent)
    e.put(std::make_unique<CompressedIntProduct>(value_));
}

void
CompressedIntProducer::endSubRun(art::SubRun& sr)
{
  std::cerr << "Holy cow, CompressedIntProducer::endSubRun is running!\n";
  if (branchType_ == art::InSubRun)
    sr.put(std::make_unique<CompressedIntProduct>(value_),
           art::subRunFragment());
}

void
CompressedIntProducer::endRun(art::Run& r)
{
  std::cerr << "Holy cow, CompressedIntProducer::endRun is running!\n";
  if (branchType_ == art::InRun)
    r.put(std::make_unique<CompressedIntProduct>(value_), art::runFragment());
}

DEFINE_ART_MODULE(CompressedIntProducer)
