//--------------------------------------------------------------------
//
// Produces an IntProduct instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/BranchType.h"

#include "fhiclcpp/types/Atom.h"

#include "art/test/TestObjects/ToyProducts.h"

#include <iostream>
#include <memory>

namespace {

  using namespace fhicl;
  struct Config {
    Atom<int> ivalue{Name("ivalue")};
    Atom<unsigned long> branchType{Name("branchType"), art::InEvent};
  };
}

namespace arttest {
  class IntProducer;
}

using arttest::IntProducer;

class arttest::IntProducer : public art::EDProducer {
public:
  using Parameters = EDProducer::Table<Config>;
  explicit IntProducer(EDProducer::Table<Config> const& p)
    : value_{p().ivalue()} // enums don't usually have a conversion from string
    , branchType_{art::BranchType(p().branchType())}
  {
    switch (branchType_) {
      case art::InEvent:
        produces<IntProduct>();
        break;
      case art::InSubRun:
        produces<IntProduct, art::InSubRun>();
        break;
      case art::InRun:
        produces<IntProduct, art::InRun>();
        break;
      default:
        throw art::Exception(art::errors::LogicError)
          << "Unknown branch type " << branchType_ << ".\n";
    }
  }

  explicit IntProducer(int i) : value_(i) { produces<IntProduct>(); }

  void produce(art::Event& e) override;
  void endSubRun(art::SubRun& sr) override;
  void endRun(art::Run& r) override;

private:
  int value_;
  art::BranchType branchType_;

}; // IntProducer

void
IntProducer::produce(art::Event& e)
{
  std::cerr << "Holy cow, IntProducer::produce is running!\n";
  if (branchType_ == art::InEvent)
    e.put(std::make_unique<IntProduct>(value_));
}

void
IntProducer::endSubRun(art::SubRun& sr)
{
  std::cerr << "Holy cow, IntProducer::endSubRun is running!\n";
  if (branchType_ == art::InSubRun)
    sr.put(std::make_unique<IntProduct>(value_), art::subRunFragment());
}

void
IntProducer::endRun(art::Run& r)
{
  std::cerr << "Holy cow, IntProducer::endRun is running!\n";
  if (branchType_ == art::InRun)
    r.put(std::make_unique<IntProduct>(value_), art::runFragment());
}

DEFINE_ART_MODULE(IntProducer)
