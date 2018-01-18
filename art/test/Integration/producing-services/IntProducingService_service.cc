#include "art/Framework/Core/ProducingService.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/types/Atom.h"

namespace {

  class IntProducingService : public art::ProducingService {
  public:
    struct Config {
      fhicl::Atom<int> ivalue{fhicl::Name("ivalue")};
      fhicl::Atom<unsigned> branchType{fhicl::Name("branchType"), art::InEvent};
    };
    using Parameters = art::ServiceTable<Config>;
    explicit IntProducingService(Parameters const&);

  private:
    void postReadRun(art::Run&) override;
    void postReadSubRun(art::SubRun&) override;
    void postReadEvent(art::Event&) override;

    int const value_;
    art::BranchType const branchType_;
  };

  IntProducingService::IntProducingService(Parameters const& p)
    : value_{p().ivalue()} // enums don't usually have a conversion from string
    , branchType_{art::BranchType(p().branchType())}
  {
    switch (branchType_) {
      case art::InEvent:
        produces<arttest::IntProduct>("", art::Persistable::No);
        break;
      case art::InSubRun:
        produces<arttest::IntProduct, art::InSubRun>("", art::Persistable::No);
        break;
      case art::InRun:
        produces<arttest::IntProduct, art::InRun>("", art::Persistable::No);
        break;
      default:
        throw art::Exception(art::errors::LogicError)
          << "Unknown branch type " << branchType_ << ".\n";
    }
  }

  void
  IntProducingService::postReadRun(art::Run& r)
  {
    if (branchType_ == art::InRun)
      r.put(std::make_unique<arttest::IntProduct>(value_), art::fullRun());
  }

  void
  IntProducingService::postReadSubRun(art::SubRun& sr)
  {
    if (branchType_ == art::InSubRun)
      sr.put(std::make_unique<arttest::IntProduct>(value_), art::fullSubRun());
  }

  void
  IntProducingService::postReadEvent(art::Event& e)
  {
    if (branchType_ == art::InEvent)
      e.put(std::make_unique<arttest::IntProduct>(value_));
  }
}

DEFINE_ART_PRODUCING_SERVICE(IntProducingService)
