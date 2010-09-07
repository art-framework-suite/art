#ifndef EngineCreator_h
#define EngineCreator_h

// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGeneratorService::createEngine()
//
// ======================================================================


// Framework support:
#include "art/Framework/Core/RandomNumberGeneratorService.h"
#include "art/Framework/Services/Registry/Service.h"
#include "art/ParameterSet/ParameterSet.h"


// ======================================================================

namespace edm {

  class EngineCreator
  {
  private:
    typedef  RandomNumberGeneratorService  RNGservice;
    typedef  RNGservice::label_t           label_t;
    typedef  RNGservice::seed_t            seed_t;
    typedef  RNGservice::base_engine_t     base_engine_t;

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
      get_seed_value( edm::ParameterSet const &  pset
                    , char              const    key [ ]       = "seed"
                    , seed_t            const    implicit_seed = -1
                    );

  private:
    static  edm::Service<edm::RandomNumberGeneratorService> &
      rng( );

  };  // EngineCreator

}  // namespace edm

#endif  // EngineCreator_h
