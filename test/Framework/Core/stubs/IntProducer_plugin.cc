

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "art/Persistency/Common/DetSetVector.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/Ref.h"
#include "art/Persistency/Common/View.h"
#include "art/Persistency/Common/RefVector.h"
#include "art/Persistency/Common/RefToBaseVector.h"
#include "test/TestObjects/ToyProducts.h"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/ParameterSet/ParameterSetDescription.h"

#include <boost/shared_ptr.hpp>

#include <iostream>

namespace edmtest {

  //--------------------------------------------------------------------
  //
  // Produces an IntProduct instance.
  //
  class IntProducer : public art::EDProducer {
  public:
    explicit IntProducer(art::ParameterSet const& p) :
      value_(p.getParameter<int>("ivalue")) {
      produces<IntProduct>();
    }
    explicit IntProducer(int i) : value_(i) {
      produces<IntProduct>();
    }
    virtual ~IntProducer() { }
    virtual void produce(art::Event& e, art::EventSetup const& c);

    static void fillDescription(art::ParameterSetDescription& iDesc,
                                std::string const& moduleLabel) {
      iDesc.setAllowAnything();

      iDesc.add<int>("ivalue", 1);
      iDesc.addUntracked<int>("uvalue", 7);
      iDesc.addOptional<int>("ovalue", 7);
      iDesc.addOptionalUntracked<int>("ouvalue", 7);

      //add a ParameterSet
      art::ParameterSetDescription bar;
      bar.add<unsigned int>("Drinks",5);
      bar.addUntracked<unsigned int>("uDrinks",5);
      bar.addOptional<unsigned int>("oDrinks",5);
      bar.addOptionalUntracked<unsigned int>("ouDrinks",5);
      iDesc.add("bar",bar);

      //add a ParameterSet
      art::ParameterSetDescription barx;
      barx.add<unsigned int>("Drinks",5);
      barx.addUntracked<unsigned int>("uDrinks",5);
      barx.addOptional<unsigned int>("oDrinks",5);
      barx.addOptionalUntracked<unsigned int>("ouDrinks",5);
      std::vector<art::ParameterSetDescription> bars;
      bars.push_back(barx);
      iDesc.add("bars",bars);

      art::ParameterDescription* parDescription;
      parDescription = iDesc.addOptional<art::ParameterSetDescription>("subpset", art::ParameterSetDescription());
      art::ParameterSetDescription* subPsetDescription =
        parDescription->parameterSetDescription();

      subPsetDescription->add<int>("xvalue", 11);
    }
  private:
    int value_;
  };

  void
  IntProducer::produce(art::Event& e, art::EventSetup const&) {
    // EventSetup is not used.
    std::cerr << "Holy cow, IntProducer::produce is running!\n";
    std::auto_ptr<IntProduct> p(new IntProduct(value_));
    e.put(p);
  }


}

using edmtest::IntProducer;
DEFINE_FWK_MODULE(IntProducer);


