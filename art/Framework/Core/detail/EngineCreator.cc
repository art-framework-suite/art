// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGenerator::createEngine()
//
// ======================================================================

#include "art/Framework/Core/detail/EngineCreator.h"
#include "fhiclcpp/ParameterSet.h"

#include <utility>

using art::detail::EngineCreator;
using fhicl::ParameterSet;

namespace {
  auto
  label_and_seed(ParameterSet const& pset)
  {
    auto const& label = pset.get<std::string>("module_label");
    // Value of -1 tells CLHEP to use its own default.
    auto const seed = pset.get<EngineCreator::seed_t>("seed", -1);
    return std::make_pair(label, seed);
  }
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(ScheduleID const sid, ParameterSet const& pset)
{
  auto const pr = label_and_seed(pset);
  return rng()->createEngine(sid, pr.first, pr.second);
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(ScheduleID const sid,
                            ParameterSet const& pset,
                            std::string const& kind_of_engine_to_make)
{
  auto const pr = label_and_seed(pset);
  return rng()->createEngine(sid, pr.first, pr.second, kind_of_engine_to_make);
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(ScheduleID const sid,
                            ParameterSet const& pset,
                            std::string const& kind_of_engine_to_make,
                            label_t const& engine_label)
{
  auto const pr = label_and_seed(pset);
  return rng()->createEngine(
    sid, pr.first, pr.second, kind_of_engine_to_make, engine_label);
}

art::ServiceHandle<art::RandomNumberGenerator>&
EngineCreator::rng()
{
  static art::ServiceHandle<art::RandomNumberGenerator> rng;
  return rng;
}

// ======================================================================
