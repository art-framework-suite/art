// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGeneratorService::createEngine()
//
// ======================================================================


#include "art/Framework/Core/EngineCreator.h"
  using edm::EngineCreator;

// C++ support:
#include <vector>


// ======================================================================


EngineCreator::base_engine_t &
  EngineCreator::createEngine( seed_t  seed )
{
  return rng()->createEngine( seed );
}

EngineCreator::base_engine_t &
  EngineCreator::createEngine( seed_t               seed
                             , std::string const &  kind_of_engine_to_make
                             )
{
  return rng()->createEngine( seed
                            , kind_of_engine_to_make
                            );
}

EngineCreator::base_engine_t &
  EngineCreator::createEngine( seed_t               seed
                             , std::string const &  kind_of_engine_to_make
                             , label_t const &      engine_label
                             )
{
  return rng()->createEngine( seed
                            , kind_of_engine_to_make
                            , engine_label
                            );
}

EngineCreator::seed_t
  EngineCreator::get_seed_value( edm::ParameterSet const &  pset
                               , char              const    key [ ]
                               , seed_t            const    implicit_seed
                               )
{
  typedef  std::vector<int>  sv_t;

  sv_t const &  explicit_seeds
    = pset.getUntrackedParameter<sv_t>( key, sv_t() );
  return explicit_seeds.empty() ? implicit_seed
                                : explicit_seeds.front();
}

edm::Service<edm::RandomNumberGeneratorService> &
  EngineCreator::rng( )
{
  static  edm::Service<edm::RandomNumberGeneratorService>  rng;
  return rng;
}
