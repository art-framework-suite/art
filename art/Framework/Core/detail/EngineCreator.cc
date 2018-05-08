// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGenerator::createEngine()
//
// ======================================================================

#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Utilities/ScheduleID.h"
#include <vector>

using art::detail::EngineCreator;

EngineCreator::base_engine_t&
EngineCreator::createEngine(ScheduleID const sid, seed_t const seed)
{
  return rng()->createEngine(sid, seed);
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(ScheduleID const sid,
                            seed_t const seed,
                            std::string const& kind_of_engine_to_make)
{
  return rng()->createEngine(sid, seed, kind_of_engine_to_make);
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(ScheduleID const sid,
                            seed_t const seed,
                            std::string const& kind_of_engine_to_make,
                            label_t const& engine_label)
{
  return rng()->createEngine(sid, seed, kind_of_engine_to_make, engine_label);
}

art::ServiceHandle<art::RandomNumberGenerator>&
EngineCreator::rng()
{
  static art::ServiceHandle<art::RandomNumberGenerator> rng;
  return rng;
}

// ======================================================================
