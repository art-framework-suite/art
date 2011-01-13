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

using arttest::IntTestAnalyzer;

class arttest::IntTestAnalyzer
  : public art::EDAnalyzer
{
public:
  IntTestAnalyzer(const fhicl::ParameterSet& conf) :
    value_(conf.get<int>("valueMustMatch")),
    moduleLabel_(conf.get<std::string>("moduleLabel"))
  { }

  void analyze(const art::Event& e)
  {
    art::Handle<IntProduct> handle;
    e.getByLabel(moduleLabel_,handle);
    if(handle->value != value_) {
      throw cet::exception("ValueMismatch")
        << "The value for \"" << moduleLabel_
        << "\" is " << handle->value
        << " but was supposed to be " << value_
        << '\n';
    }
  }

private:
  int value_;
  std::string moduleLabel_;
};  // IntTestAnalyzer

DEFINE_ART_MODULE(IntTestAnalyzer);
