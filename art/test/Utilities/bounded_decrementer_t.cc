#define BOOST_TEST_MODULE (parent_path_t)
#include "boost/test/unit_test.hpp"

#include "art/Utilities/bounded_decrementer.h"

BOOST_AUTO_TEST_SUITE(bounded_decrementer_t)

BOOST_AUTO_TEST_CASE(bounded_decrementer_nonnegative)
{
  art::bounded_decrementer d{2};
  // Pre-decrement
  BOOST_TEST(d == 2);
  --d;
  BOOST_TEST(d == 1);

  // Post-decrement
  auto e = d--;
  BOOST_TEST(e == 1);
  BOOST_TEST(d == 0);
  --e;

  // Both at zero now
  BOOST_TEST(e == d);

  // Decrementing again is a no-op
  --e;
  --d;
  BOOST_TEST(e == d);
  BOOST_TEST(e == 0);
}

BOOST_AUTO_TEST_CASE(bounded_decrementer_negative)
{
  // Starting with a negative number results in a no-op
  art::bounded_decrementer d{-1};
  BOOST_TEST(d == -1);
  --d;
  BOOST_TEST(d == -1);
  auto e = d--;
  BOOST_TEST(e == -1);
  --e;
  BOOST_TEST(e == d);
}

BOOST_AUTO_TEST_SUITE_END()
