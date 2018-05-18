#ifndef art_Framework_Core_detail_EngineCreator_h
#define art_Framework_Core_detail_EngineCreator_h

// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGenerator::createEngine()
//
// MT-TODO:
//
//   Once we adopt multiple module types (legacy, one, per-schedule,
//   and global), then the steps that are necessary for creating
//   engines will depend on the given module type.  If a user
//   specifies:
//
//     createEngine(1237);
//
//   then N instances of the specified engine should be created, one
//   for each of N schedules, regardless of the module type.
//
//   A decision must be made if each instance should receive the same
//   seed, or different ones.  I suspect they should be different so
//   as to avoid undesirable over-sampling of fluctuations.
//
// ======================================================================

#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/ScheduleID.h"

namespace fhicl {
  class ParameterSet;
}

namespace art {
  namespace detail {
    class EngineCreator {
      using RNGservice = RandomNumberGenerator;

    public:
      // --- CLHEP engine characteristics:
      using base_engine_t = CLHEP::HepRandomEngine;
      using label_t = RNGsnapshot::label_t;
      using seed_t = long;
      using engine_state_t = RNGsnapshot::engine_state_t;

      // We keep a default c'tor for backwards compatibility.  It will
      // go away once the default constructors for EDProducer and
      // EDFilter are removed.
      EngineCreator() = default;
      explicit EngineCreator(std::string const& label, ScheduleID sid);

      base_engine_t& createEngine(seed_t seed);
      base_engine_t& createEngine(seed_t seed,
                                  std::string const& kind_of_engine_to_make);
      base_engine_t& createEngine(seed_t seed,
                                  std::string const& kind_of_engine_to_make,
                                  label_t const& engine_label);

    private:
      static ServiceHandle<RandomNumberGenerator>& rng();
      void requireValid(); // Can remove once default c'tor is gone.
      std::string const moduleLabel_{};
      ScheduleID const sid_{};
    }; // EngineCreator
  }    // detail
} // art

#endif /* art_Framework_Core_detail_EngineCreator_h */

// Local Variables:
// mode: c++
// End:
