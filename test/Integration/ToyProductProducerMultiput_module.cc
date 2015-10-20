//--------------------------------------------------------------------
//
// Produces a ToyProductProducer instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/BranchType.h"

#include "fhiclcpp/types/Atom.h"

#include "test/TestObjects/ToyProducts.h"

namespace fhicl { class ParameterSet; }

namespace {

  using namespace fhicl;
  struct Config {
    Atom<unsigned long> branchType { Name("branchType") };
  };

}

namespace arttest {

  class ToyProductProducerMultiput : public art::EDProducer {

    art::BranchType branchType_;
    
  public:

    using Parameters = EDProducer::Table<Config>;
    explicit ToyProductProducerMultiput( Parameters const& p )
      : branchType_( art::BranchType( p().branchType() ) )
    {
      switch (branchType_) {
      case art::InEvent:
        produces<IntProduct>();
        break;
      case art::InSubRun:
        produces<IntProduct, art::InSubRun>();
        break;
      case art::InRun:
        produces<IntProduct, art::InRun>();
        break;
      default:
        throw art::Exception(art::errors::LogicError)
          << "Unknown branch type "
          << branchType_
          << ".\n";
      }
    }

    void produce(art::Event &e) override 
    {
      if ( branchType_ != art::InEvent ) return;
      e.put( std::make_unique<IntProduct>(1) );
      e.put( std::make_unique<IntProduct>(2) );
    }

    void endSubRun(art::SubRun &sr) override 
    {
      if ( branchType_ != art::InSubRun ) return;
      sr.put( std::make_unique<IntProduct>(3) );
      sr.put( std::make_unique<IntProduct>(4) );
    }

    void endRun(art::Run &r) override 
    {
      if ( branchType_ != art::InRun ) return;
      r.put( std::make_unique<IntProduct>(5) );
      r.put( std::make_unique<IntProduct>(6) );
    }
    
  };

}

DEFINE_ART_MODULE(arttest::ToyProductProducerMultiput)
