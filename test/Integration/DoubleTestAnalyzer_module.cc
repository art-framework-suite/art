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
  class DoubleTestAnalyzer;
}

class arttest::DoubleTestAnalyzer
  : public art::EDAnalyzer
{
public:
  DoubleTestAnalyzer(const fhicl::ParameterSet& conf) :
    value_(conf.get<double>("valueMustMatch")),
    moduleLabel_(conf.get<std::string>("input_label"))
  { }

  void analyze(const art::Event& e)
  {
    art::Handle<DoubleProduct> handle;
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
  double value_;
  std::string moduleLabel_;
};  // DoubleTestAnalyzer

DEFINE_ART_MODULE(arttest::DoubleTestAnalyzer);
