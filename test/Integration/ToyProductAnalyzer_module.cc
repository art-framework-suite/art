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

    virtual ~ToyProductAnalyzer(){}

    virtual void beginRun   ( const art::Run   & r  ) override {
      art::Handle<StringProduct> rh1;
      assert(r.getByLabel( inputLabel_, "bgnRun", rh1));

      art::Handle<StringProduct> rh2;
      assert(r.getByLabel( inputLabel_, "", rh2));
    }

    virtual void beginSubRun( const art::SubRun& sr ) override {
      art::Handle<StringProduct> srh1;
      assert(sr.getByLabel( inputLabel_, "bgnSubRun", srh1));

      art::Handle<StringProduct> srh2;
      assert(sr.getByLabel( inputLabel_, "", srh2));
    }

    virtual void analyze    ( const art::Event & e  ) override {
      art::InputTag const tag(inputLabel_, "" );
      e.getValidHandle<StringProduct>(tag);
    }

    virtual void endSubRun  ( const art::SubRun& sr ) override {
      art::Handle<StringProduct> srh;
      assert(sr.getByLabel( inputLabel_, "endSubRun", srh));
    }

    virtual void endRun     ( const art::Run   & r  ) override {
      art::Handle<StringProduct> rh;
      assert(r.getByLabel( inputLabel_, "endRun", rh));
    }

  };  // ToyProductAnalyzer

}

DEFINE_ART_MODULE(arttest::ToyProductAnalyzer)
