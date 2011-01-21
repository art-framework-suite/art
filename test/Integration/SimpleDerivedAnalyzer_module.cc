// ======================================================================
//
// SimpleDerivedAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
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
  , nvalues_   ( p.get<int>("nvalues") )
  { }

  virtual ~SimpleDerivedAnalyzer() { }
  virtual void analyze( art::Event const & e );

private:
  std::string inputLabel_;
  unsigned    nvalues_;

};  // SimpleDerivedAnalyzer

void
  SimpleDerivedAnalyzer::analyze( art::Event const & e )
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

}  // analyze()


// ----------------------------------------------------------------------

DEFINE_ART_MODULE(SimpleDerivedAnalyzer);

// ======================================================================
