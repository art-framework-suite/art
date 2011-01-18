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
  , value_      ( p.get<int        >("value") )
  , nvalues_    ( p.get<unsigned   >("nvalues") )
  { }

  void analyze( art::Event const & e )
  {
    std::vector<int const *> ptrs;
    unsigned sz = e.getView(moduleLabel_, ptrs);

    if( sz != nvalues_ ) {
      throw cet::exception("SizeMismatch")
        << "Expected a view of size " << nvalues_
        << " but the obtained size is " << sz
        << '\n';

    for( unsigned k = 0; k != sz; ++k )
      if( *ptrs[k] != value_ )
        throw cet::exception("ValueMismatch")
          << "At position " << k
          << " expected value " << value_
          << " but obtained " << *ptrs[k]
          << '\n';
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
