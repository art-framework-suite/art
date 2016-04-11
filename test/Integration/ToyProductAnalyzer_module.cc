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

    virtual void beginRun   ( const art::Run   & r  ) override {
      (void)r.getValidHandle<StringProduct>(art::InputTag{inputLabel_, "bgnRun"});
      (void)r.getValidHandle<StringProduct>(art::InputTag{inputLabel_});
    }

    virtual void beginSubRun( const art::SubRun& sr ) override {
      (void)sr.getValidHandle<StringProduct>(art::InputTag{inputLabel_, "bgnSubRun"});
      (void)sr.getValidHandle<StringProduct>(art::InputTag{inputLabel_});
    }

    virtual void analyze    ( const art::Event & e  ) override {
      (void)e.getValidHandle<StringProduct>(inputLabel_);
    }

    virtual void endSubRun  ( const art::SubRun& sr ) override {
      (void)sr.getValidHandle<StringProduct>(art::InputTag{inputLabel_,"endSubRun"});
    }

    virtual void endRun     ( const art::Run   & r  ) override {
      (void)r.getValidHandle<StringProduct>(art::InputTag{inputLabel_,"endRun"});
    }

  };  // ToyProductAnalyzer

}

DEFINE_ART_MODULE(arttest::ToyProductAnalyzer)
