#include "art/Framework/Core/ModuleBase.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/PerThread.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace CLHEP {
  class HepRandomEngine;
}

using namespace hep::concurrency;
using namespace std;

namespace art {

  ModuleBase::~ModuleBase() noexcept = default;
  ModuleBase::ModuleBase() = default;

  ModuleDescription const&
  ModuleBase::moduleDescription() const
  {
    return md_;
  }

  ScheduleID
  ModuleBase::scheduleID() const
  {
    return scheduleID_;
  }

  void
  ModuleBase::setModuleDescription(ModuleDescription const& md)
  {
    md_ = md;
  }

  void
  ModuleBase::setScheduleID(ScheduleID const si)
  {
    scheduleID_ = si;
  }

  CLHEP::HepRandomEngine&
  ModuleBase::createEngine(long const seed)
  {
    // Cannot use scheduleID_ because it is not set before the user's
    // module constructor is called.
    auto const sid = PerThread::instance()->getCPC().scheduleID();
    return ServiceHandle<RandomNumberGenerator> {}
    ->createEngine(sid, seed);
  }

  CLHEP::HepRandomEngine&
  ModuleBase::createEngine(long const seed,
                           std::string const& kind_of_engine_to_make)
  {
    // Cannot use scheduleID_ because it is not set before the user's
    // module constructor is called.
    auto const sid = PerThread::instance()->getCPC().scheduleID();
    return ServiceHandle<RandomNumberGenerator> {}
    ->createEngine(sid, seed, kind_of_engine_to_make);
  }

  CLHEP::HepRandomEngine&
  ModuleBase::createEngine(long const seed,
                           std::string const& kind_of_engine_to_make,
                           std::string const& engine_label)
  {
    // Cannot use scheduleID_ because it is not set before the user's
    // module constructor is called.
    auto const sid = PerThread::instance()->getCPC().scheduleID();
    return ServiceHandle<RandomNumberGenerator> {}
    ->createEngine(sid, seed, kind_of_engine_to_make, engine_label);
  }

  long
  ModuleBase::get_seed_value(fhicl::ParameterSet const& pset,
                             char const key[],
                             long const implicit_seed)
  {
    auto const& explicit_seeds = pset.get<std::vector<int>>(key, {});
    return explicit_seeds.empty() ? implicit_seed : explicit_seeds.front();
  }

  array<vector<ProductInfo>, NumBranchTypes> const&
  ModuleBase::getConsumables() const
  {
    return consumables_;
  }

  void
  ModuleBase::sortConsumables()
  {
    // Now that we know we have seen all the consumes declarations,
    // sort the results for fast lookup later.
    for (auto& vecPI : consumables_) {
      sort(vecPI.begin(), vecPI.end());
    }
  }

} // namespace art
