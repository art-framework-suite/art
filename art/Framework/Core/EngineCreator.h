#ifndef EngineCreator_h
#define EngineCreator_h

// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGenerator::createEngine()
//
// ======================================================================

#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {

  class EngineCreator
  {
  private:
    typedef  RandomNumberGenerator      RNGservice;
    typedef  RNGservice::label_t        label_t;
    typedef  RNGservice::seed_t         seed_t;
    typedef  RNGservice::base_engine_t  base_engine_t;

  public:
    // --- Engine establishment:
    base_engine_t &
      createEngine( seed_t  seed );
    base_engine_t &
      createEngine( seed_t               seed
                  , std::string const &  kind_of_engine_to_make
                  );
    base_engine_t &
      createEngine( seed_t               seed
                  , std::string const &  kind_of_engine_to_make
                  , label_t const &      engine_label
                  );

    // --- seed access
    seed_t
      get_seed_value( fhicl::ParameterSet const &  pset
                    , char              const    key [ ]       = "seed"
                    , seed_t            const    implicit_seed = -1
                    );

  private:
    static  art::ServiceHandle<art::RandomNumberGenerator> &
      rng( );

  };  // EngineCreator

}  // art

// ======================================================================

#endif
