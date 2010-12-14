
// This test module will look for IntProducts in Events.
// The number of IntProducts and their InputTags (label,
// instance, process) must be configured.

// One can also configure an expected value for the sum of
// all the values in the IntProducts that are found.  Note
// that an IntProduct is just a test product that simply
// contains a single integer.

// If the products are not found, then an exception is thrown.
// If the sum does not match there is an error message and
// an abort.

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/ParameterSet/InputTag.h"
#include "test/TestObjects/ToyProducts.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Framework/Core/Event.h"

#include <iostream>
#include <vector>

#include "art/Framework/Core/Frameworkfwd.h"

namespace arttest
{
  class TestFindProduct : public art::EDAnalyzer
  {
  public:

    explicit TestFindProduct(art::ParameterSet const& pset);
    virtual ~TestFindProduct();

    virtual void analyze(art::Event const& e, art::EventSetup const& es);
    virtual void endJob();

  private:

    std::vector<art::InputTag> inputTags_;
    int expectedSum_;
    int sum_;
  }; // class TestFindProduct

  //--------------------------------------------------------------------
  //
  // Implementation details

  TestFindProduct::TestFindProduct(art::ParameterSet const& pset) :
    inputTags_(pset.getUntrackedParameter<std::vector<art::InputTag> >("inputTags")),
    expectedSum_(pset.getUntrackedParameter<int>("expectedSum", 0)),
    sum_(0)
  {
  }

  TestFindProduct::~TestFindProduct() {}

  void
  TestFindProduct::analyze(art::Event const& e, art::EventSetup const& es)
  {
    art::Handle<IntProduct> h;

    for (std::vector<art::InputTag>::const_iterator iter = inputTags_.begin(),
	   iEnd = inputTags_.end();
         iter != iEnd;
         ++iter) {
      e.getByLabel(*iter, h);
      sum_ += h->value;
    }
  }

  void
  TestFindProduct::endJob()
  {
    std::cout << "TestFindProduct sum = " << sum_ << std::endl;
    if (expectedSum_ != 0 && sum_ != expectedSum_) {
      std::cerr << "TestFindProduct: Sum of test object values does not equal expected value" << std::endl;
      abort();
    }
  }
} // namespace arttest

using arttest::TestFindProduct;
DEFINE_FWK_MODULE(TestFindProduct);
