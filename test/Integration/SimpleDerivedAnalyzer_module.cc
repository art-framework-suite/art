// ======================================================================
//
// SimpleDerivedAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/PtrVector.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace arttest {
  class SimpleDerivedAnalyzer;
}

using arttest::SimpleDerivedAnalyzer;

//--------------------------------------------------------------------
//
// Produces a SimpleProduct product instance.
//
class arttest::SimpleDerivedAnalyzer
  : public art::EDAnalyzer
{
public:
  typedef  std::vector<arttest::SimpleDerived>  SimpleDerivedProduct;

  explicit SimpleDerivedAnalyzer( fhicl::ParameterSet const & p )
  : inputLabel_( p.get<std::string>("input_label") )
  , inputLabel2_( p.get<std::string>("input_label2") )
  , nvalues_   ( p.get<int>("nvalues") )
  { }

  virtual ~SimpleDerivedAnalyzer() { }
  virtual void analyze( art::Event const & e )
  {
    test_getView(e);
    test_PtrVector(e);
  }

  void test_getView( art::Event const & e ) const;
  void test_PtrVector( art::Event const & e ) const;

private:
  std::string inputLabel_;
  std::string inputLabel2_;
  unsigned    nvalues_;

};  // SimpleDerivedAnalyzer

void
  SimpleDerivedAnalyzer::test_getView( art::Event const & e ) const
{
  int event_num = e.id().event();
  {  // test using base class
    std::vector<Simple const *> ptrs;
    unsigned sz = e.getView(inputLabel_, "derived", ptrs);

    if( sz != nvalues_ ) {
      std::cerr
        << "SizeMismatch expected a view of size " << nvalues_
        << " but the obtained size is " << sz
        << '\n';
      throw cet::exception("SizeMismatch")
        << "Expected a view of size " << nvalues_
        << " but the obtained size is " << sz
        << '\n';
    }

    for( unsigned k = 0; k != sz; ++k ) {
      if( ptrs[k]->key != sz-k+event_num ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected key " << sz-k+event_num
          << " but obtained " << ptrs[k]->key
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected key " << sz-k+event_num
          << " but obtained " << ptrs[k]->key
          << '\n';
      }
      double expect = 1.5 * k + 100.0;
      if( ptrs[k]->value != expect ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected value " << expect
          << " but obtained " << ptrs[k]->value
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected value " << expect
          << " but obtained " << ptrs[k]->value
          << '\n';
      }
      if( ptrs[k]->dummy() != 16.25 ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << ptrs[k]->dummy()
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << ptrs[k]->dummy()
          << '\n';
      }
    }
  }

  {  // test using derived class
    std::vector<SimpleDerived const *> ptrs;
    unsigned sz = e.getView(inputLabel_, "derived", ptrs);

    if( sz != nvalues_ ) {
      std::cerr
        << "SizeMismatch expected a view of size " << nvalues_
        << " but the obtained size is " << sz
        << '\n';
      throw cet::exception("SizeMismatch")
        << "Expected a view of size " << nvalues_
        << " but the obtained size is " << sz
        << '\n';
    }

    int value_ = e.id().event();
    for( unsigned k = 0; k != sz; ++k ) {
      if( ptrs[k]->key != sz-k+event_num ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected key " << sz-k+event_num
          << " but obtained " << ptrs[k]->key
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected key " << sz-k+event_num
          << " but obtained " << ptrs[k]->key
          << '\n';
      }
      double expect = 1.5 * k + 100.0;
      if( ptrs[k]->value != expect ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected value " << expect
          << " but obtained " << ptrs[k]->value
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected value " << expect
          << " but obtained " << ptrs[k]->value
          << '\n';
      }
      if( ptrs[k]->dummy() != 16.25 ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << ptrs[k]->dummy()
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << ptrs[k]->dummy()
          << '\n';
      }
    }
  }

}  // test_getView()

void
  SimpleDerivedAnalyzer::test_PtrVector( art::Event const & e ) const
{
  int event_num = e.id().event();
  {  // test using derived class
    typedef  art::PtrVector<arttest::SimpleDerived>  product_t;
    art::Handle<product_t>  h;
    e.getByLabel(inputLabel2_, h);

    unsigned sz = h->size();
    if( sz != 16 ) {
      std::cerr
        << "SizeMismatch expected a PtrVector of size " << 16
        << " but the obtained size is " << sz
        << '\n';
      throw cet::exception("SizeMismatch")
        << "Expected a PtrVector of size " << 16
        << " but the obtained size is " << sz
        << '\n';
    }

    for( product_t::const_iterator b = h->begin()
                                 , e = h->end(); b!= e; ++b ) {
      int k = b - h->begin();
      if( (*b)->key != sz-k+event_num ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected key " << sz-k+event_num
          << " but obtained " << (*b)->key
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << (b - h->begin())
          << " expected key " << sz-k+event_num
          << " but obtained " << (*b)->key
          << '\n';
      }
      double expect = 1.5 * k + 100.0;
      if( (*b)->value != expect ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected value " << expect
          << " but obtained " << (*b)->value
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << (b - h->begin())
          << " expected value " << expect
          << " but obtained " << (*b)->value
          << '\n';
      }
      if( (*b)->dummy() != 16.25 ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << (*b)->dummy()
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected dummy value " << 16.25
          << " but obtained " << (*b)->dummy()
          << '\n';
      }
    }
  }

}  // test_PtrVector()


// ----------------------------------------------------------------------

DEFINE_ART_MODULE(SimpleDerivedAnalyzer);

// ======================================================================
