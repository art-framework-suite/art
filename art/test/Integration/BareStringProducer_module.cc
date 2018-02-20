//--------------------------------------------------------------------
//
// Produces an IntProduct instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/test/Integration/RunTimeProduces.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <string>

namespace arttest {
  class BareStringProducer;
}

using arttest::BareStringProducer;

class arttest::BareStringProducer : public art::EDProducer {
public:
  struct Config {
    fhicl::Atom<std::string> value{fhicl::Name{"value"}};
    fhicl::Atom<unsigned long> branchType{fhicl::Name("branchType"),
                                          art::InEvent};
  };

  using Parameters = art::EDProducer::Table<Config>;
  explicit BareStringProducer(Parameters const& p)
    : value_{p().value()}, branchType_{art::BranchType(p().branchType())}
  {
    art::test::run_time_produces<std::string>(this, branchType_);
  }

private:

  void beginRun(art::Run& r) override;
  void beginSubRun(art::SubRun& sr) override;
  void produce(art::Event& e) override;

  std::string value_;
  art::BranchType branchType_;

}; // BareStringProducer

void
BareStringProducer::beginRun(art::Run& r)
{
  if (branchType_ != art::InRun) return;
  r.put(std::make_unique<std::string>(value_));
}

void
BareStringProducer::beginSubRun(art::SubRun& sr)
{
  if (branchType_ != art::InSubRun) return;
  sr.put(std::make_unique<std::string>(value_));
}

void
BareStringProducer::produce(art::Event& e)
{
  if (branchType_ != art::InEvent) return;
  e.put(std::make_unique<std::string>(value_));
}

DEFINE_ART_MODULE(BareStringProducer)
