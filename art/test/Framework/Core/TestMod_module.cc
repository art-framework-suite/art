#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDProducer.h"
#include <cppunit/extensions/HelperMacros.h>

class TestMod : public art::EDProducer
{
public:
   explicit TestMod(fhicl::ParameterSet const& p);

   void produce(art::Event&) override;
};

TestMod::TestMod(fhicl::ParameterSet const&)
{ produces<int>(); }

void TestMod::produce(art::Event&)
{
   art::CurrentProcessingContext const* p = currentContext();
   CPPUNIT_ASSERT( p != 0 );
   CPPUNIT_ASSERT( p->moduleDescription() != 0 );
   CPPUNIT_ASSERT( p->moduleLabel() != 0 );
}

DEFINE_ART_MODULE(TestMod)
