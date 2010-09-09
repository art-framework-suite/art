// ======================================================================
//
// RandomNumberSaver_plugin:  Store state of RandomNumberGeneratorService
//                            into the event.
//
// ======================================================================


// --- Framework support:
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/RandomNumberGeneratorService.h"
#include "art/Framework/Services/Registry/Service.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/ParameterSet/ParameterSetDescription.h"

// --- C++ support:
#include <memory>


// --- Contents:
namespace edm {
  class RandomNumberSaver;
}
using edm::RandomNumberSaver;


// ======================================================================


class edm::RandomNumberSaver
  : public EDProducer
{
  typedef  RandomNumberGeneratorService  RNGservice;

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

  // --- Specification:
  static void
    fillDescription( ParameterSetDescription & iDesc
                   , label_t           const & moduleLabel
                   )
  {
    iDesc.setAllowAnything();
  }

private:
  bool  debug_;

};  // RandomNumberSaver


RandomNumberSaver::RandomNumberSaver( ParameterSet const & pset )
  : EDProducer( )
  , debug_    ( pset.getUntrackedParameter<bool>( "debug"
                                                , false
              )                                 )
{
  produces<snapshot_t>();
}


void
  RandomNumberSaver::produce( Event & event )
{
  using std::auto_ptr;

  Service<RNGservice>  rng;
  auto_ptr<snapshot_t>  product_ptr( new snapshot_t( rng->accessSnapshot_()
                                   )               );

  event.put( product_ptr );

  if( debug_ )  {
    rng->print_();
  }
}  // produce()


// ======================================================================


DEFINE_FWK_MODULE(RandomNumberSaver);
