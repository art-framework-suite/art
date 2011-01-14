// ======================================================================
//
// IntVectorAnalyzer
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
//#include "test/TestObjects/ToyProducts.h"
#include <boost/shared_ptr.hpp>
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
    e.getView(moduleLabel_, ptrs);
    #if 0
    art::Handle<intvector_t> handle;
    e.getByLabel(moduleLabel_,handle);
    if( handle->value != value_ ) {
      throw cet::exception("ValueMismatch")
        << "The value for \"" << moduleLabel_
        << "\" is " << handle->value
        << " but was supposed to be " << value_
        << '\n';
    }
    #endif
  }

private:
  std::string moduleLabel_;
  int         value_;
  unsigned    nvalues_;

};  // IntTestAnalyzer

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(arttest::IntTestAnalyzer);

// ======================================================================
