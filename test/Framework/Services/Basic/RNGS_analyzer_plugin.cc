// ======================================================================
//
// Test the RandomNumberGeneratorService from an EDAnalyzer's viewpoint
//
// ======================================================================


// Framework support:
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/Framework/Core/RandomNumberGeneratorService.h"

// CLHEP support:
#include "CLHEP/Random/RandFlat.h"

// C++ support:
#include <iostream>


namespace test {

  class RNGS_analyzer
    : public edm::EDAnalyzer
  {
  public:
    static int
      get_seed( edm::ParameterSet const & pset
              , char const                key [ ] = "seed"
              )
    {
      static  int const  default_seed  =  13579;
      int const  seed  =  pset.getUntrackedParameter<int>( key
                                                         , default_seed
                                                         );
      std::cerr << key <<" is " << seed << '\n';
      return seed;
    }

    explicit
      RNGS_analyzer( edm::ParameterSet const & pset )
      : this_event_number( 0u )
      , flat             ( createEngine(get_seed_value(pset)) )
    { }

    virtual
      ~RNGS_analyzer( )
    { }

    virtual void
      analyze( edm::Event const &, edm::EventSetup const & )
    {
      ++this_event_number;

      std::cerr << "Firing flat: ";
      for ( int i = 0; i != 5; ++i ) {
        std::cerr << " " << flat.fire();
      }
      std::cerr << '\n';
    }  // analyze()

  private:
    unsigned         this_event_number;
    CLHEP::RandFlat  flat;

  };  // RNGS_analyzer

}  // namespace test


#include "art/Framework/Core/MakerMacros.h"
using test::RNGS_analyzer;
DEFINE_FWK_MODULE(RNGS_analyzer);
