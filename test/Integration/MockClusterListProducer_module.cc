// Produces a MockClusterList starting from a vector<SimpleDerived>

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include "test/TestObjects/MockCluster.h"
#include <iostream>
#include <memory>
#include <vector>

namespace arttest {
  class MockClusterListProducer;
}

using arttest::MockClusterListProducer;

// ----------------------------------------------------------------------

class arttest::MockClusterListProducer
  : public art::EDProducer
{
public:
   typedef  std::vector<arttest::SimpleDerived>     input_t;
   typedef  arttest::MockClusterList product_t;

   explicit MockClusterListProducer( fhicl::ParameterSet const & p )
      :
      input_label_( p.get<std::string>("input_label") ),
      nvalues_( p.get<int>("nvalues") )
   {
      produces<product_t>();
   }

   virtual ~MockClusterListProducer() { }

   virtual void produce( art::Event & e );

private:
   std::string  input_label_;
   unsigned nvalues_;

};  // MockClusterListProducer

// ----------------------------------------------------------------------

void
MockClusterListProducer::produce( art::Event& e )
{
   std::cerr << "MockClusterListProducer::produce is running!\n";

   art::Handle<input_t> h;
   e.getByLabel(input_label_, "derived", h);

   std::auto_ptr<product_t> prod( new product_t );
   arttest::MockCluster c1;
   c1.skew = 1;
   for( int k = 0; k < (nvalues_ / 2); ++k ) {
      art::Ptr<SimpleDerived> p(h, k, true);
      c1.cells.push_back(p);
   }
   c1.eNum = e.id().event();
   prod->push_back(c1);

   arttest::MockCluster c2;
   c2.skew = 2;
   for( int k = nvalues_ / 2; k < nvalues_; ++k ) {
      art::Ptr<SimpleDerived> p(h, k, true);
      c2.cells.push_back(p);
   }
   c2.eNum = e.id().event() + 1;
   prod->push_back(c2);

   e.put(prod);
}

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(MockClusterListProducer);

// ======================================================================
