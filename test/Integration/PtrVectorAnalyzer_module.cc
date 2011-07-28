// ======================================================================
//
// PtrVectorAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/PtrVector.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>
#include <vector>

namespace arttest {
  class PtrVectorAnalyzer;
}

// ----------------------------------------------------------------------

class arttest::PtrVectorAnalyzer
  : public art::EDAnalyzer
{
public:
  typedef  art::PtrVector<int>  product_t;

  PtrVectorAnalyzer( fhicl::ParameterSet const & p )
  : input_label_( p.get<std::string>("input_label") )
  , nvalues_    ( p.get<unsigned   >("nvalues") )
  { }

  void analyze( art::Event const & e )
  {
    art::Handle<product_t> h;
    e.getByLabel(input_label_, h);

    size_t sz = h->size();
    if( sz != nvalues_ ) {
      throw cet::exception("SizeMismatch")
        << "Expected a PtrVector of size " << nvalues_
        << " but the obtained size is " << sz
        << '\n';
    }

    int value = e.id().event();
    size_t count = 0;
    for( product_t::const_iterator b = h->begin()
            , e = h->end(); b!= e; ++b, ++value, ++count ) {
      if( **b != value ) {
        throw cet::exception("ValueMismatch")
          << "At position " << (b - h->begin())
          << " expected value " << value
          << " but obtained " << **b
          << '\n';
      }
    }
    if (count != sz) {
       throw cet::exception("CountMismatch")
          << "Expected to iterate over "
          << sz << " values, but found "
          << count
          << '\n';
    }

    // Make a copy of the PtrVector, so we can call sort on it.
    product_t local(*h);
    // Make sure we're not sorted yet...
    sz = local.size();
    assert (sz > 1);

    local.sort();
    assert(sz == local.size());
    for (size_t i = 1; i != sz; ++i)
      assert(*local[i-1] < *local[i]);

    std::greater<int> gt;
    local.sort(gt);
    assert(sz == local.size());
    for (size_t i = 1; i != sz; ++i)
      assert(*local[i-1] > *local[i]);

  }  // analyze()

private:
  std::string input_label_;
  unsigned    nvalues_;

};  // PtrVectorAnalyzer

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(arttest::PtrVectorAnalyzer);

// ======================================================================
