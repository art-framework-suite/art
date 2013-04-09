//--------------------------------------------------------------------
//
// Produces a CompressedIntProduct instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include "test/TestObjects/ToyProducts.h"

#include <iostream>

namespace arttest {
  class CompressedIntProducer;
}

using arttest::CompressedIntProducer;

class arttest::CompressedIntProducer
  : public art::EDProducer
{
public:
  explicit CompressedIntProducer( fhicl::ParameterSet const& p )
    : value_( p.get<int>("ivalue") ),
      // enums don't usually have a conversion from string
      branchType_( art::BranchType(p.get<unsigned long>("branchType", art::InEvent)) )
  {
    switch (branchType_) {
    case art::InEvent:
      produces<CompressedIntProduct>();
      break;
    case art::InSubRun:
      produces<CompressedIntProduct, art::InSubRun>();
      break;
    case art::InRun:
      produces<CompressedIntProduct, art::InRun>();
      break;
    default:
      throw art::Exception(art::errors::LogicError)
        << "Unknown branch type "
        << branchType_
        << ".\n";
    }
  }

  explicit CompressedIntProducer( int i )
  : value_(i)
  {
    produces<CompressedIntProduct>();
  }

  virtual ~CompressedIntProducer() { }

  virtual void produce( art::Event& e );
  virtual void endSubRun( art::SubRun & sr );
  virtual void endRun( art::Run& r );

private:

  template <typename PUTTER>
  void put(PUTTER & p);

  int value_;
  art::BranchType branchType_;

};  // CompressedIntProducer

void
CompressedIntProducer::produce( art::Event& e )
{
  std::cerr << "Holy cow, CompressedIntProducer::produce is running!\n";
  if (branchType_ == art::InEvent) put(e);
}

void
CompressedIntProducer::endSubRun( art::SubRun& sr )
{
  std::cerr << "Holy cow, CompressedIntProducer::endSubRun is running!\n";
  if (branchType_ == art::InSubRun) put(sr);
}

void
CompressedIntProducer::endRun( art::Run& r )
{
  std::cerr << "Holy cow, CompressedIntProducer::endRun is running!\n";
  if (branchType_ == art::InRun) put(r);
}

template <typename PUTTER>
void
CompressedIntProducer::put(PUTTER & p) {
  p.put(std::unique_ptr<CompressedIntProduct>(
    new CompressedIntProduct(value_)));
}

DEFINE_ART_MODULE(CompressedIntProducer)

