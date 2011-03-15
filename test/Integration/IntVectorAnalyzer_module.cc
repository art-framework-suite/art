// ======================================================================
//
// IntVectorAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>
#include <string>
#include <vector>

namespace arttest {
  class IntTestAnalyzer;
}

// ----------------------------------------------------------------------

class arttest::IntTestAnalyzer
  : public art::EDAnalyzer
{
public:
  typedef  std::vector<int>  intvector_t;

  IntTestAnalyzer( fhicl::ParameterSet const & p )
  : moduleLabel_( p.get<std::string>("input_label") )
  , nvalues_    ( p.get<unsigned   >("nvalues") )
  { }

  void analyze( art::Event const & e )
  {
    std::vector<int const *> ptrs;
    unsigned sz = e.getView(moduleLabel_, ptrs);

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
    for( int k = 0; k != sz; ++k ) {
      if( *ptrs[k] != value_+k ) {
        std::cerr
          << "ValueMismatch at position " << k
          << " expected value " << value_+k
          << " but obtained " << *ptrs[k]
          << '\n';
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected value " << value_+k
          << " but obtained " << *ptrs[k]
          << '\n';
      }
    }
  }  // analyze()

private:
  std::string moduleLabel_;
  int         value_;
  unsigned    nvalues_;

};  // IntTestAnalyzer

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(arttest::IntTestAnalyzer);

// ======================================================================
