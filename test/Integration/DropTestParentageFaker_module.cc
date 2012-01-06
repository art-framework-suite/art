////////////////////////////////////////////////////////////////////////
// Class:       DropTestParentageFaker
// Module Type: producer
// File:        DropTestParentageFaker_module.cc
//
// Generated at Thu Jan  5 17:44:19 2012 by Chris Green using artmod
// from art v1_00_06.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Ptr.h"

#include <cpp0x/memory>

namespace arttest {
  class DropTestParentageFaker;
}

class arttest::DropTestParentageFaker : public art::EDProducer {
public:
  explicit DropTestParentageFaker(fhicl::ParameterSet const & p);
  virtual ~DropTestParentageFaker();

  virtual void produce(art::Event & e);


private:
  std::string inputLabel_;
};


arttest::DropTestParentageFaker::DropTestParentageFaker(fhicl::ParameterSet const & p)
  :
  inputLabel_(p.get<std::string>("input_label"))
{
  produces<std::string>();
}

arttest::DropTestParentageFaker::~DropTestParentageFaker()
{
}

void arttest::DropTestParentageFaker::produce(art::Event & e)
{
  art::Handle<art::Ptr<std::string> > sh;
  // Force this product to be a parent of our child.
  e.getByLabel(inputLabel_, sh);
  e.put(std::auto_ptr<std::string>(new std::string("Child")));
}

DEFINE_ART_MODULE(arttest::DropTestParentageFaker)
