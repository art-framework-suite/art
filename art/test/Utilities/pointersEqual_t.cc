// Test of pointersEqual
#define BOOST_TEST_MODULE (pointersEqual_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Utilities/pointersEqual.h"
BOOST_AUTO_TEST_SUITE(pointersEqual_t)

class A {};
class B {};
class C : public A, public B {};
struct D : public C {};
class E : public A {};

BOOST_AUTO_TEST_CASE(basic_types)
{
  int i = 3;
  int j = 3;
  int* ip1 = &i;
  int* ip2 = &i;
  double p = i;
  BOOST_CHECK(art::pointersEqual(ip1, ip2));
  BOOST_CHECK(art::pointersEqual(&i, ip1));
  BOOST_CHECK(art::pointersEqual(&i, ip2));
  BOOST_CHECK_THROW(art::pointersEqual(&i, &p), art::Exception);
  BOOST_CHECK(!art::pointersEqual(&i, &j));
}

BOOST_AUTO_TEST_CASE(inheritance)
{
  D d1;
  D* pd1d1(&d1);
  D* pd1d2(&d1);
  A* pd1a1(&d1);
  B* pd1b1(&d1);
  D d2;
  A a1;
  B b1;
  E e1;
  BOOST_CHECK(art::pointersEqual(pd1d1, pd1d2));
  BOOST_CHECK(art::pointersEqual(pd1d1, pd1a1));
  BOOST_CHECK(art::pointersEqual(pd1d1, pd1b1));
  BOOST_CHECK_THROW(art::pointersEqual(pd1a1, pd1b1), art::Exception);
  BOOST_CHECK_THROW(art::pointersEqual(&a1, &b1), art::Exception);
  BOOST_CHECK(!art::pointersEqual(pd1a1, &a1));
  BOOST_CHECK(!art::pointersEqual(pd1a1, &e1));
  BOOST_CHECK(!art::pointersEqual(&d1, &d2));
}

BOOST_AUTO_TEST_CASE(constness)
{
  D d1;
  A const* da1(&d1);
  BOOST_CHECK(art::pointersEqual(&d1, da1));
}

BOOST_AUTO_TEST_SUITE_END()
