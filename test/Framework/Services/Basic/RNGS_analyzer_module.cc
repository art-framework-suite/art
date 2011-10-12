// ======================================================================
//
// Test the RandomNumberGenerator from an EDAnalyzer's viewpoint
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "fhiclcpp/ParameterSet.h"
#include "CLHEP/Random/RandFlat.h"
#include <iostream>

namespace test {

  class RNGS_analyzer
      : public art::EDAnalyzer {
  public:
    static int
    get_seed(fhicl::ParameterSet const & pset
             , char const                key [ ] = "seed"
            ) {
      static  int const  default_seed  =  13579;
      int const  seed  =  pset.get<int>(key
                                        , default_seed
                                       );
      std::cerr << key << " is " << seed << '\n';
      return seed;
    }

    explicit
    RNGS_analyzer(fhicl::ParameterSet const & pset)
      : this_event_number(0u)
      , flat(createEngine(get_seed_value(pset)))
    { }

    virtual
    ~RNGS_analyzer()
    { }

    virtual void
    analyze(art::Event const &, art::EventSetup const &) {
      ++this_event_number;
      std::cerr << "Firing flat: ";
      for (int i = 0; i != 5; ++i) {
        std::cerr << " " << flat.fire();
      }
      std::cerr << '\n';
    }  // analyze()

  private:
    unsigned         this_event_number;
    CLHEP::RandFlat  flat;

  };  // RNGS_analyzer

}  // test


#include "art/Framework/Core/ModuleMacros.h"
using test::RNGS_analyzer;
DEFINE_ART_MODULE(RNGS_analyzer);
