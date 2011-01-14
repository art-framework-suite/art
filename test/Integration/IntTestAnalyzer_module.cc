/*----------------------------------------------------------------------

Toy EDProducers and EDProducts for testing purposes only.

----------------------------------------------------------------------*/

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/TestObjects/ToyProducts.h"
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <string>

namespace arttest {
  class IntTestAnalyzer;
}

class arttest::IntTestAnalyzer
  : public art::EDAnalyzer
{
public:
  IntTestAnalyzer(const fhicl::ParameterSet& conf) :
    value_(conf.get<int>("valueMustMatch")),
    moduleLabel_(conf.get<std::string>("input_label")),
    require_presence_(conf.get<bool>("require_presence", true))    
  { }

  void analyze(const art::Event& e)
  {
    if (require_presence_)
      require_product_presence(e);
    else
      require_product_absence(e);
  }

  void require_product_presence(art::Event const& e)
  {
    art::Handle<IntProduct> handle;
    e.getByLabel(moduleLabel_, handle);
    if(handle->value != value_) {
      throw cet::exception("ValueMismatch")
        << "The value for \"" << moduleLabel_
        << "\" is " << handle->value
        << " but was supposed to be " << value_
        << '\n';
    }
  }

  void require_product_absence(art::Event const& e)
  {
    art::Handle<IntProduct> handle;
    try
      {
	e.getByLabel(moduleLabel_, handle);
	throw cet::exception("WronglyFound")
	  << "The product for \"" << moduleLabel_
	  << "\" is supposed to be absent, but is present\n";
      }
    catch (art::Exception& ex)
      {
	assert(ex.categoryCode() == art::errors::ProductNotFound);
      }
    // Other exception types will propagate out, and
    // indicate failure...    
  }


private:
  int value_;
  std::string moduleLabel_;
  bool require_presence_;
};  // IntTestAnalyzer

DEFINE_ART_MODULE(arttest::IntTestAnalyzer);
