//--------------------------------------------------------------------
// Produces an IntProduct instance.
//--------------------------------------------------------------------

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/Integration/RunTimeProduces.h"
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

} // namespace

namespace arttest {
  class IntProducer;
}

using arttest::IntProducer;

class arttest::IntProducer : public art::SharedProducer {
public:
  using Parameters = Table<Config>;
  explicit IntProducer(Parameters const& p)
    : art::SharedProducer{p}
    , value_{p().ivalue()} // enums don't usually have a conversion from string
    , branchType_{art::BranchType(p().branchType())}
  {
    art::test::run_time_produces<IntProduct>(this, branchType_);
    async<art::InEvent>();
  }

private:
  void produce(art::Event& e, art::Services const&) override;
  void endSubRun(art::SubRun& sr, art::Services const&) override;
  void endRun(art::Run& r, art::Services const&) override;

  int const value_;
  art::BranchType const branchType_;
}; // IntProducer

void
IntProducer::produce(art::Event& e, art::Services const&)
{
  if (branchType_ == art::InEvent)
    e.put(std::make_unique<IntProduct>(value_));
}

void
IntProducer::endSubRun(art::SubRun& sr, art::Services const&)
{
  if (branchType_ == art::InSubRun)
    sr.put(std::make_unique<IntProduct>(value_), art::subRunFragment());
}

void
IntProducer::endRun(art::Run& r, art::Services const&)
{
  if (branchType_ == art::InRun)
    r.put(std::make_unique<IntProduct>(value_), art::runFragment());
}

DEFINE_ART_MODULE(IntProducer)
