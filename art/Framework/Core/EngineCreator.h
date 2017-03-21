#ifndef art_Framework_Core_EngineCreator_h
#define art_Framework_Core_EngineCreator_h

// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGenerator::createEngine()
//
// MT-FIXME:
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
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {

  class EngineCreator {
    using RNGservice = RandomNumberGenerator;
    using label_t = RNGservice::label_t;
    using seed_t = RNGservice::seed_t;
    using base_engine_t = RNGservice::base_engine_t;

  public:

    base_engine_t& createEngine(seed_t seed);
    base_engine_t& createEngine(seed_t seed, std::string const& kind_of_engine_to_make);
    base_engine_t& createEngine(seed_t seed, std::string const& kind_of_engine_to_make, label_t const& engine_label);

    // --- seed access
    seed_t get_seed_value(fhicl::ParameterSet const& pset,
                          char const key [] = "seed",
                          seed_t const implicit_seed = -1);

  private:
    static art::ServiceHandle<art::RandomNumberGenerator>& rng();

  };  // EngineCreator

}  // art

// ======================================================================

#endif /* art_Framework_Core_EngineCreator_h */

// Local Variables:
// mode: c++
// End:
