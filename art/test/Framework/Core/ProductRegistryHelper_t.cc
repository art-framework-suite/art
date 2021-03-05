#define BOOST_TEST_MODULE (ProductRegistryHelper_t)
#include "boost/test/unit_test.hpp"

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "canvas/Utilities/Exception.h"

using namespace art;

namespace {
  class matches {
    std::string msg_;
  public:
    explicit matches(std::string msg)
      : msg_{move(msg)}
    {}

    bool operator()(art::Exception const& e) const
    {
      return e.explain_self().find(msg_) != std::string::npos;
    }
  };
}

BOOST_AUTO_TEST_SUITE(ProductRegistryHelper_t)

BOOST_AUTO_TEST_CASE(produces)
{
  ProductRegistryHelper prh{product_creation_mode::produces};
  prh.produces<int>();
  BOOST_CHECK_EXCEPTION((prh.reconstitutes<int, InEvent>("label", "instance")),
                        art::Exception,
                        matches{"'produces' should have been called instead"});
}

BOOST_AUTO_TEST_CASE(reconstitutes)
{
  ProductRegistryHelper prh{product_creation_mode::reconstitutes};
  prh.reconstitutes<int, InEvent>("label", "instance");
  BOOST_CHECK_EXCEPTION(prh.produces<int>(),
                        art::Exception,
                        matches{"'reconstitutes' should have been called instead"});
}

BOOST_AUTO_TEST_SUITE_END()
