// ======================================================================
//
// PtrVectorAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Handle.h"
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
  : art::EDAnalyzer(p)
  , input_label_( p.get<std::string>("input_label") )
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
    for ( const auto ptr : *h ) {
      if( *ptr != value ) {
        throw cet::exception("ValueMismatch")
          << "At position " << count
          << " expected value " << value
          << " but obtained " << *ptr
          << '\n';
      }
      ++value;
      ++count;
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

    // Make a new PtrVector so we can range-insert into it.
    product_t insert_test;
    auto half_size = h->size() / 2;
    insert_test.insert(insert_test.begin(), h->cbegin(), h->cbegin() + half_size);
    auto it = insert_test.insert(insert_test.end(), h->cbegin() + half_size, h->cend());
    assert(it == insert_test.begin() + half_size);
    assert(insert_test.size() == h->size());
    it = insert_test.insert(it, h->cbegin(), h->end());
    assert(it ==  insert_test.begin() + half_size);
    assert(insert_test.size() == h->size() * 2);
  }  // analyze()

private:
  std::string input_label_;
  unsigned    nvalues_;

};  // PtrVectorAnalyzer

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(arttest::PtrVectorAnalyzer)

// ======================================================================
