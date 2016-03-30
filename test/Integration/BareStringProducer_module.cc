//--------------------------------------------------------------------
//
// Produces an IntProduct instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Handle.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <memory>
#include <string>

namespace arttest {
  class BareStringProducer;
}

using arttest::BareStringProducer;

class arttest::BareStringProducer
  : public art::EDProducer
{
public:
  explicit BareStringProducer( fhicl::ParameterSet const& p )
    : value_( p.get<std::string>("value") )
  {
    produces<std::string>();
  }

  explicit BareStringProducer( std::string const &s )
  : value_(s)
  {
    produces<std::string>();
  }

  void produce( art::Event& e ) override;

private:
  std::string value_;
};  // BareStringProducer

void
BareStringProducer::produce( art::Event& e )
{
  std::cerr << "Holy cow, BareStringProducer::produce is running!\n";
  std::unique_ptr<std::string> p(new std::string(value_));
  e.put(std::move(p));
}

DEFINE_ART_MODULE(BareStringProducer)
