
/*----------------------------------------------------------------------

Toy EDProducers and EDProducts for testing purposes only.

----------------------------------------------------------------------*/

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/Handle.h"
#include "test/TestObjects/ToyProducts.h"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"

#include "fhiclcpp/ParameterSet.h"

#include <boost/shared_ptr.hpp>

#include <iostream>

  //--------------------------------------------------------------------
  //
  class IntTestAnalyzer : public art::EDAnalyzer {
  public:
    IntTestAnalyzer(const fhicl::ParameterSet& conf) :
      value_(conf.get<int>("valueMustMatch")),
      moduleLabel_(conf.get<std::string>("moduleLabel")) {
      }

    void analyze(const art::Event& e) {
      art::Handle<IntProduct> handle;
      e.getByLabel(moduleLabel_,handle);
      if(handle->value != value_) {
	throw cet::exception("ValueMismatch")
	  << "The value for \"" << moduleLabel_ << "\" is "
	  << handle->value << " but it was supposed to be " << value_;
      }
    }
  private:
    int value_;
    std::string moduleLabel_;
  };

DEFINE_FWK_MODULE(IntTestAnalyzer);

