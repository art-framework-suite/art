// ======================================================================
//
// RandomNumberSaver_plugin:  Store state of the RandomNumberGenerator
//                            service into the event.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace art {
  class RandomNumberSaver;
}

using namespace fhicl;
using art::RandomNumberSaver;

// ======================================================================

class art::RandomNumberSaver
  : public EDProducer
{
  using RNGservice = RandomNumberGenerator;

public:
  // --- Characteristics:
  using label_t    = RNGservice::label_t;
  using snapshot_t = RNGservice::snapshot_t;

  // --- Configuration
  struct Config {
    Atom<bool> debug { Key("debug"), false };
  };

  // --- C'tor/d'tor:
  using Parameters = EDProducer::Table<Config>;
  explicit  RandomNumberSaver( Parameters const & );
  virtual  ~RandomNumberSaver()  { }

  // --- Production:
  virtual void produce( Event & );

private:
  bool  debug_;

};  // RandomNumberSaver

// ======================================================================

RandomNumberSaver::
RandomNumberSaver( Parameters const & config )
  : EDProducer( )
  , debug_    ( config().debug() )
{
  produces<snapshot_t>();
}

// ----------------------------------------------------------------------

void
  RandomNumberSaver::
  produce( Event & event )
{

  ServiceHandle<RNGservice>  rng;
  event.put(std::unique_ptr<snapshot_t>(new snapshot_t( rng->accessSnapshot_())));

  if( debug_ )  {
    rng->print_();
  }
}  // produce()

// ======================================================================

DEFINE_ART_MODULE(RandomNumberSaver)

// ======================================================================
