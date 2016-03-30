//--------------------------------------------------------------------
//
// Analyzes products inserted by ToyProductProducer
//   - tests products with identical names in different branch types
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"

#include <cassert>

namespace arttest {

  class ToyProductAnalyzer : public art::EDAnalyzer {

    std::string inputLabel_;

  public:

    explicit ToyProductAnalyzer( fhicl::ParameterSet const& pset) :
      art::EDAnalyzer(pset),
      inputLabel_( pset.get<std::string>("inputLabel") )
    {}

    void beginRun   ( const art::Run   & r  ) override {
      r.getValidHandle<StringProduct>(art::InputTag{inputLabel_, "bgnRun"});
      r.getValidHandle<StringProduct>(art::InputTag{inputLabel_});
    }

    void beginSubRun( const art::SubRun& sr ) override {
      sr.getValidHandle<StringProduct>(art::InputTag{inputLabel_, "bgnSubRun"});
      sr.getValidHandle<StringProduct>(art::InputTag{inputLabel_});
    }

    void analyze    ( const art::Event & e  ) override {
      e.getValidHandle<StringProduct>(inputLabel_);
    }

    void endSubRun  ( const art::SubRun& sr ) override {
      sr.getValidHandle<StringProduct>(art::InputTag{inputLabel_,"endSubRun"});
    }

    void endRun     ( const art::Run   & r  ) override {
      r.getValidHandle<StringProduct>(art::InputTag{inputLabel_,"endRun"});
    }

  };  // ToyProductAnalyzer

}

DEFINE_ART_MODULE(arttest::ToyProductAnalyzer)
