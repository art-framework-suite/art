#include "art/Framework/Core/ModuleBase.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/Globals.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueueChain.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <array>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace CLHEP {

class HepRandomEngine;

} // namespace CLHEP

using namespace hep::concurrency;
using namespace std;

namespace art {

ModuleBase::
~ModuleBase()
{
}

ModuleBase::
ModuleBase()
  : md_{}
  , streamIndex_{}
  , moduleThreadingType_{}
  , resourceNames_{}
  , chain_{}
  //, requireConsumes_{false}
  , consumables_{}
  //, missingConsumes_{}
{
}

ModuleDescription const&
ModuleBase::
moduleDescription() const
{
  return md_;
}

int
ModuleBase::
streamIndex() const
{
  return streamIndex_;
}

void
ModuleBase::
setModuleDescription(ModuleDescription const& md)
{
  md_ = md;
}

void
ModuleBase::
setStreamIndex(int si)
{
  streamIndex_ = si;
}

SerialTaskQueueChain*
ModuleBase::
serialTaskQueueChain() const
{
  return chain_.get();
}

ModuleThreadingType
ModuleBase::
moduleThreadingType() const
{
  return moduleThreadingType_;
}

void
ModuleBase::
uses(std::string const& resourceName)
{
  auto posAndSuccess = resourceNames_.emplace(resourceName);
  auto newName = posAndSuccess.second;
  if (newName) {
    SharedResourcesRegistry::instance()->registerSharedResource(resourceName);
  }
}

// Warning: Cannot call from the ctor for stream modules because streamIndex_ is not set yet!
CLHEP::HepRandomEngine&
ModuleBase::
createEngine(long const seed)
{
  return ServiceHandle<RandomNumberGenerator>{}->createEngine(streamIndex_, seed);
}

// Warning: Cannot call from the ctor for stream modules because streamIndex_ is not set yet!
CLHEP::HepRandomEngine&
ModuleBase::
createEngine(long const seed, std::string const& kind_of_engine_to_make)
{
  return ServiceHandle<RandomNumberGenerator>{}->createEngine(streamIndex_, seed, kind_of_engine_to_make);
}

// Warning: Cannot call from the ctor for stream modules because streamIndex_ is not set yet!
CLHEP::HepRandomEngine&
ModuleBase::
createEngine(long const seed, std::string const& kind_of_engine_to_make, std::string const& engine_label)
{
  return ServiceHandle<RandomNumberGenerator>{}->createEngine(streamIndex_, seed, kind_of_engine_to_make, engine_label);
}

long
ModuleBase::
get_seed_value(fhicl::ParameterSet const& pset, char const key[], long const implicit_seed)
{
  auto const& explicit_seeds = pset.get<std::vector<int>>(key, {});
  return explicit_seeds.empty() ? implicit_seed : explicit_seeds.front();
}

//ModuleBase::ProductInfo::
//ProductInfo(ConsumableType const consumableType, TypeID const& tid)
// : consumableType_{consumableType}
// , typeID_{tid}
// , label_{}
// , instance_{}
// , process_{}
//{
//}

//ModuleBase::ProductInfo::
//ProductInfo(ConsumableType const consumableType, TypeID const& tid, std::string const& label, std::string const& instance,
//            std::string const& process)
// : consumableType_{consumableType}
// , typeID_{tid}
// , label_{label}
// , instance_{instance}
// , process_{process}
//{
//}

//bool
//operator<(ProductInfo const& a, ProductInfo const& b)
//{
//  return tie(a.consumableType_, a.typeID_, a.label_, a.instance_, a.process_) <
//         tie(b.consumableType_, b.typeID_, b.label_, b.instance_, b.process_);
//}

//bool
//ModuleBase::
//getRequireConsumes() const
//{
//  return requireConsumes_;
//}

//void
//ModuleBase::
//setRequireConsumes(bool val) const
//{
//  requireConsumes_ = val;
//}

array<vector<ProductInfo>, NumBranchTypes> const&
ModuleBase::
getConsumables() const
{
  return consumables_;
}

void
ModuleBase::
sortConsumables()
{
  // Now that we know we have seen all the consumes declarations,
  // sort the results for fast lookup later.
  for (auto& vecPI : consumables_) {
    sort(vecPI.begin(), vecPI.end());
  }
}

//std::array<std::set<ProductInfo>, NumBranchTypes>&
//ModuleBase::
//getMissingConsumes();
//{
//  return missingConsumes_
//}

//void
//ModuleBase::
//prepareForJob(fhicl::ParameterSet const& pset)
//{
//  pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
//}

//void
//ModuleBase::
//showMissingConsumes() const
//{
//  if (all_of(missingConsumes_.cbegin(), missingConsumes_.cend(), [](auto const& perBranch) { return perBranch.empty(); })) {
//    // Nothing was missing, all done.
//    return;
//  }
//  constexpr cet::HorizontalRule rule{60};
//  mf::LogPrint log{"MTdiagnostics"};
//  log << '\n' << rule('=') << '\n'
//      << "The following consumes (or mayConsume) statements are missing from\n"
//      << "module label: '"
//      << module_context(md_)
//      << rule('-') << '\n';
//  size_t i = 0;
//  for (auto const& perBT : missingConsumes_) {
//    for (auto const& pi : perBT) {
//      log << "  " << assemble_consumes_statement(static_cast<BranchType>(i), pi) << '\n';
//    }
//    ++i;
//  }
//  log << rule('=');
//}

} // namespace art

