#define BOOST_TEST_MODULE (ProductRegistryHelper_t)
#include "boost/test/unit_test.hpp"

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

using namespace art;

namespace {
  class matches {
    std::string msg_;

  public:
    explicit matches(std::string msg) : msg_{std::move(msg)} {}

    bool
    operator()(art::Exception const& e) const
    {
      return e.explain_self().find(msg_) != std::string::npos;
    }
  };

  auto const dummy_pset_id = fhicl::ParameterSet{}.id();
  ModuleDescription const md{dummy_pset_id,
                             "_NAMEERROR_",
                             "_LABELERROR_",
                             ModuleThreadingType::legacy,
                             ProcessConfiguration{"Process", dummy_pset_id, {}},
                             true /*isEmulated*/};
}

BOOST_AUTO_TEST_SUITE(ProductRegistryHelper_t)

BOOST_AUTO_TEST_CASE(produces)
{
  ProductRegistryHelper prh{product_creation_mode::produces};
  prh.produces<int>();
  BOOST_CHECK_EXCEPTION((prh.reconstitutes<int, InEvent>("label", "instance")),
                        Exception,
                        matches{"'produces' should have been called instead"});
}

BOOST_AUTO_TEST_CASE(reconstitutes)
{
  ProductRegistryHelper prh{product_creation_mode::reconstitutes};
  prh.reconstitutes<int, InEvent>("label", "instance");
  BOOST_CHECK_EXCEPTION(
    prh.produces<int>(),
    art::Exception,
    matches{"'reconstitutes' should have been called instead"});
}

BOOST_AUTO_TEST_CASE(reconstitutes_different_module_labels)
{
  ProductRegistryHelper prh{product_creation_mode::reconstitutes};
  prh.reconstitutes<int, InEvent>("label1");
  prh.reconstitutes<int, InEvent>("label2");
  BOOST_CHECK_NO_THROW(prh.fillDescriptions(md));
}

BOOST_AUTO_TEST_CASE(reconstitutes_assns_reversed_types)
{
  ProductRegistryHelper prh{product_creation_mode::reconstitutes};
  prh.reconstitutes<Assns<int, double>, InEvent>("label");
  prh.reconstitutes<Assns<double, int>, InEvent>("label");
  BOOST_CHECK_EXCEPTION(
    prh.fillDescriptions(md),
    Exception,
    matches{"That friendly name has already been registered for this module."});
}

BOOST_AUTO_TEST_SUITE_END()
