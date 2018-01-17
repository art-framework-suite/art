//===================================================================
// The "LastDoubleProducingService" is used in combination with the
// "IntProducingService" to test the availability of products.  Since
// IntProducingService comes before LastDoubleProducingService
// alphabetically, the callbacks for this service will be called after
// those of IntProducingService.  In that case, the
// IntProducingService will have created its IntProduct already, but
// the system is designed so that any products produced by a service
// are inaccessible to other ProducingServices at the same callback
// level (i.e. postReadRun).  This service tests that behavior.
// ===================================================================

#include "art/Framework/Core/ProducingService.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/types/Atom.h"

#include <cassert>

namespace {

  class LastDoubleProducingService : public art::ProducingService {
  public:
    struct Config {
      fhicl::Atom<std::string> input_label{fhicl::Name{"input_label"}};
      fhicl::Atom<double> value{fhicl::Name("value")};
      fhicl::Atom<unsigned> branchType{fhicl::Name("branchType"), art::InEvent};
    };
    using Parameters = art::ServiceTable<Config>;
    explicit LastDoubleProducingService(Parameters const&);

  private:
    void postReadRun(art::Run&) override;
    void postReadSubRun(art::SubRun&) override;
    void postReadEvent(art::Event&) override;

    art::InputTag const tag_;
    double const value_;
    art::BranchType const branchType_;
  };

  LastDoubleProducingService::LastDoubleProducingService(Parameters const& p)
    : tag_{p().input_label()}
    , value_{p().value()} // enums don't usually have a conversion from string
    , branchType_{art::BranchType(p().branchType())}
  {
    switch (branchType_) {
      case art::InEvent:
        produces<arttest::DoubleProduct>("", art::Persistable::No);
        break;
      case art::InSubRun:
        produces<arttest::DoubleProduct, art::InSubRun>("",
                                                        art::Persistable::No);
        break;
      case art::InRun:
        produces<arttest::DoubleProduct, art::InRun>("", art::Persistable::No);
        break;
      default:
        throw art::Exception(art::errors::LogicError)
          << "Unknown branch type " << branchType_ << ".\n";
    }
  }

  void
  LastDoubleProducingService::postReadRun(art::Run& r)
  {
    if (branchType_ != art::InRun) return;

    // Cannot access produced products from other producing services
    art::Handle<arttest::IntProduct> h;
    assert(!r.getByLabel(tag_, h));
    r.put(std::make_unique<arttest::DoubleProduct>(value_), art::fullRun());
  }

  void
  LastDoubleProducingService::postReadSubRun(art::SubRun& sr)
  {
    if (branchType_ != art::InSubRun) return;

    // Cannot access produced products from other producing services
    art::Handle<arttest::IntProduct> h;
    assert(!sr.getByLabel(tag_, h));
    sr.put(std::make_unique<arttest::DoubleProduct>(value_),
           art::fullSubRun());
  }

  void
  LastDoubleProducingService::postReadEvent(art::Event& e)
  {
    if (branchType_ != art::InEvent) return;

    // Cannot access produced products from other producing services
    art::Handle<arttest::IntProduct> h;
    assert(!e.getByLabel(tag_, h));
    e.put(std::make_unique<arttest::DoubleProduct>(value_));
  }
}

DEFINE_ART_PRODUCING_SERVICE(LastDoubleProducingService)
