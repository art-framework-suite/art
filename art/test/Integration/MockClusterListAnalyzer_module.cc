// ======================================================================
//
// MockClusterListAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/TestObjects/MockCluster.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace arttest {
  class MockClusterListAnalyzer;
}

using arttest::MockClusterListAnalyzer;

//--------------------------------------------------------------------
//
// Produces a SimpleProduct product instance.
//
class arttest::MockClusterListAnalyzer
  : public art::EDAnalyzer
{
public:
   typedef  std::vector<arttest::SimpleDerived>  SimpleDerivedProduct;

   explicit MockClusterListAnalyzer( fhicl::ParameterSet const & p )
      : art::EDAnalyzer(p)
      , inputLabel_( p.get<std::string>("input_label") )
      , nvalues_   ( p.get<int>("nvalues") )
   { }

   void analyze( art::Event const & e ) override;

private:
   std::string inputLabel_;
   unsigned    nvalues_;

};  // MockClusterListAnalyzer

void
  MockClusterListAnalyzer::analyze( art::Event const & e )
{
   int event_num = e.id().event();
   typedef arttest::MockClusterList  product_t;
   art::Handle<product_t>  h;
   e.getByLabel(inputLabel_, h);

   assert(h.isValid());
   unsigned psz = h->size();
   assert(psz == 2);
   for(product_t::const_iterator
          pb = h->begin(),
          pi = pb,
          pe = h->end();
       pi != pe;
       ++pi) {
      int pk = pi - pb;
      size_t csz = pi->cells.size();
      if (pk == 0) {
         assert(csz == nvalues_ / 2);
      } else {
         assert(csz == nvalues_ - (nvalues_ / 2)); // Remainder
      }
      assert(pi->skew == pk + 1);
      // Try to access the first one directly.
      art::Ptr<SimpleDerived> sd0 = pi->cells[0];
      assert(sd0.get() != nullptr);

      // Now use the iterator to loop over the PtrVector.
      size_t cell_count = 0;
      for (product_t::value_type::CellList::const_iterator
              cb = pi->cells.begin(),
              ci = cb,
              ce = pi->cells.end();
           ci != ce;
           ++ci,
           ++cell_count) {
         int ck = ci - cb;
         art::Ptr<SimpleDerived> ptsd = *ci;
         assert((unsigned)ptsd->key == 2 * csz - (ck + pk * csz) + event_num);
         assert(ptsd->value == 1.5 * (ck + pk * csz) + 100.0);
         assert(ptsd->dummy() == 16.25);
      }
      assert(cell_count == csz);
      assert(pi->eNum == (unsigned)(event_num + pk));
   }
}  // test_PtrVector()


// ----------------------------------------------------------------------

DEFINE_ART_MODULE(MockClusterListAnalyzer)

// ======================================================================
