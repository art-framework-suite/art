// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (Selector_t)
#include "boost/test/unit_test.hpp"

#include "art/Framework/Principal/Selector.h"

using namespace art;

BOOST_AUTO_TEST_SUITE(Selector_t)

BOOST_AUTO_TEST_CASE(composed_selector_with_assignment)
{
  ProductInstanceNameSelector const s1("instance");
  ModuleLabelSelector const s2("moduleLabel");
  Selector s3{s1};
  s3 = Selector{s3 && s2};
}

BOOST_AUTO_TEST_CASE(composed_selector_with_not)
{
  ProductInstanceNameSelector const s1{"instance"};
  ModuleLabelSelector const s2{"moduleLabel"};
  Selector const s3 [[maybe_unused]]{s1 && !s2};
}

BOOST_AUTO_TEST_CASE(composed_selector_with_not_rvalue)
{
  Selector const selector
    [[maybe_unused]]{ProductInstanceNameSelector{"instance"} &&
                     !ModuleLabelSelector{"moduleLabel"}};
}

BOOST_AUTO_TEST_SUITE_END()
