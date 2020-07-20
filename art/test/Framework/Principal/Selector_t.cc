// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (Selector_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Principal/Selector.h"

BOOST_AUTO_TEST_SUITE(Selector_t)

BOOST_AUTO_TEST_CASE(composed_selector_with_assignment)
{
  art::ProductInstanceNameSelector const s1("instance");
  art::ModuleLabelSelector const s2("moduleLabel");
  art::Selector s3{s1};
  s3 = art::Selector{s3 && s2};
}

BOOST_AUTO_TEST_CASE(composed_selector_with_not)
{
  art::ProductInstanceNameSelector const s1{"instance"};
  art::ModuleLabelSelector const s2{"moduleLabel"};
  art::Selector const s3 [[maybe_unused]]{s1 && !s2};
}

BOOST_AUTO_TEST_SUITE_END()
