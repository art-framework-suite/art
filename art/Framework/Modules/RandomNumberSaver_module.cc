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
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class RandomNumberSaver;
}

using art::RandomNumberSaver;
using fhicl::ParameterSet;

// ======================================================================

class art::RandomNumberSaver
  : public EDProducer
{
  typedef  RandomNumberGenerator  RNGservice;

public:
  // --- Characteristics:
  typedef  RNGservice::label_t     label_t;
  typedef  RNGservice::snapshot_t  snapshot_t;

  // --- C'tor/d'tor:
  explicit  RandomNumberSaver( ParameterSet const & );
  virtual  ~RandomNumberSaver()  { }

  // --- Production:
  virtual void
    produce( Event & );

private:
  bool  debug_;

};  // RandomNumberSaver

// ======================================================================

RandomNumberSaver::
RandomNumberSaver( ParameterSet const & pset )
  : EDProducer( )
  , debug_    ( pset.get<bool>("debug", false) )
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
