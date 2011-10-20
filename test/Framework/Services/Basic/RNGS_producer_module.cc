// ======================================================================
//
// Test the RandomNumberGenerator from an EDProducer's viewpoint
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "fhiclcpp/ParameterSet.h"
#include "CLHEP/Random/RandFlat.h"
#include <iostream>

namespace test {

  class RNGS_producer
    : public art::EDProducer
  {
  public:
    explicit
      RNGS_producer( fhicl::ParameterSet const & pset )
      : this_event_number( 0u )
      , flat1            ( createEngine( get_seed_value(pset,"seed1")
                                       , "JamesRandom"
                                       , "engine_1"
                         )             )
      , flat2            ( createEngine( get_seed_value(pset,"seed2")
                                       , "JamesRandom"
                                       , "engine_2"
                         )             )
    {
      produces<product_t>( product_label() );
    }

    virtual
      ~RNGS_producer( )
    { }

    virtual void
      produce( art::Event & ev, art::EventSetup const & )
    {
      const int  N  =  6;
      ++this_event_number;

      product_t  sum  =  product_t();
      std::cerr << "Accumulation: ";
      for ( int i = 0; i != N; ++i ) {
        sum += (flat1.fire() - flat2.fire());
        std::cerr << " " << sum;
      }
      std::cerr << '\n';

      product_t  avg  =  sum / N;
      std::cerr << "Average:  " << avg << '\n';

      ev.put( std::auto_ptr<product_t>( new product_t(avg) )
            , product_label()
            );
    }  // produce()

  private:
    typedef  double  product_t;
    static  char const *
      product_label( )
    {
      static  char const  product_label [ ]  =  "accumulation";
      return product_label;
    }

    unsigned         this_event_number;
    CLHEP::RandFlat  flat1
                  ,  flat2;

    static int
      get_seed( fhicl::ParameterSet const & pset
              , char const                key [ ] = "seed"
              )
    {
      static  int const  default_seed  =  13579;
      int const  seed  =  pset.get<int>( key
                                                         , default_seed
                                                         );
      std::cerr << key <<" is " << seed << '\n';
      return seed;
    }

  };  // RNGS_producer

}  // test

#include "art/Framework/Core/ModuleMacros.h"
using test::RNGS_producer;
DEFINE_ART_MODULE(RNGS_producer)
