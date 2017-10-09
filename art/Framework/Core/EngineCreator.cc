// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGenerator::createEngine()
//
// ======================================================================

#include "art/Framework/Core/EngineCreator.h"
#include "art/Utilities/ScheduleID.h"
#include <vector>

namespace {
  // MT-TODO: Placeholder until we're multi-threaded.
  auto
  placeholder_schedule_id()
  {
    return art::ScheduleID::first();
  }
}

using art::EngineCreator;

// ======================================================================

EngineCreator::base_engine_t&
EngineCreator::createEngine(seed_t const seed)
{
  return rng()->createEngine(placeholder_schedule_id(), seed);
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(seed_t const seed,
                            std::string const& kind_of_engine_to_make)
{
  return rng()->createEngine(
    placeholder_schedule_id(), seed, kind_of_engine_to_make);
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(seed_t const seed,
                            std::string const& kind_of_engine_to_make,
                            label_t const& engine_label)
{
  return rng()->createEngine(
    placeholder_schedule_id(), seed, kind_of_engine_to_make, engine_label);
}

EngineCreator::seed_t
EngineCreator::get_seed_value(fhicl::ParameterSet const& pset,
                              char const key[],
                              seed_t const implicit_seed)
{
  auto const& explicit_seeds = pset.get<std::vector<int>>(key, {});
  return explicit_seeds.empty() ? implicit_seed : explicit_seeds.front();
}

art::ServiceHandle<art::RandomNumberGenerator>&
EngineCreator::rng()
{
  static art::ServiceHandle<art::RandomNumberGenerator> rng;
  return rng;
}

// ======================================================================
