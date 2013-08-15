#define BOOST_TEST_MODULE ( InputTag_t )
#include "boost/test/auto_unit_test.hpp"
#include "boost/test/output_test_stream.hpp"

#include "art/Utilities/InputTag.h"

#include <string>

BOOST_AUTO_TEST_SUITE ( InputTag_t )
BOOST_AUTO_TEST_CASE ( InputTag_default_ctor )
{
  art::InputTag t;
  std::string empty;
  BOOST_CHECK_EQUAL( t.label(), empty);
  BOOST_CHECK_EQUAL( t.instance(), empty);
  BOOST_CHECK_EQUAL( t.process(), empty);
}

BOOST_AUTO_TEST_CASE ( InputTag_from_string )
{
  std::string empty;
  std::string mylabel("thisIsMyLabel");
  art::InputTag t(mylabel);
  BOOST_CHECK_EQUAL( t.label(), mylabel);
  BOOST_CHECK_EQUAL( t.instance(), empty);
  BOOST_CHECK_EQUAL( t.process(), empty);
}

BOOST_AUTO_TEST_CASE ( InputTag_from_two_strings )
{
  std::string empty;
  std::string mylabel("thisIsMylabel");
  std::string myinstance("someinstance");
  art::InputTag t(mylabel, myinstance);
  BOOST_CHECK_EQUAL( t.label(), mylabel);
  BOOST_CHECK_EQUAL( t.instance(), myinstance);
  BOOST_CHECK_EQUAL( t.process(), empty);
}

BOOST_AUTO_TEST_CASE ( InputTag_from_three_strings )
{
  std::string mylabel("thisIsMylabel");
  std::string myinstance("someinstance");
  std::string myprocess("aprocess");
  art::InputTag t(mylabel, myinstance, myprocess);
  BOOST_CHECK_EQUAL( t.label(), mylabel);
  BOOST_CHECK_EQUAL( t.instance(), myinstance);
  BOOST_CHECK_EQUAL( t.process(), myprocess);
}

BOOST_AUTO_TEST_CASE ( InputTag_from_one_cstring )
{
  const char* mylabel = "mylabel";
  std::string empty;
  art::InputTag t(mylabel);
  BOOST_CHECK_EQUAL( t.label(), std::string(mylabel));
  BOOST_CHECK_EQUAL( t.instance(), empty);
  BOOST_CHECK_EQUAL( t.process(), empty);
}

std::string grabLabel(art::InputTag const& t)
{
  return t.label();
}

BOOST_AUTO_TEST_CASE ( InputTag_convert_from_string )
{
  std::string mylabel("alabel");
  std::string result = grabLabel(mylabel);
  BOOST_CHECK_EQUAL(mylabel, result);
}

BOOST_AUTO_TEST_CASE ( InputTag_convert_from_cstring )
{
  const char* mylabel = "alabel";
  std::string result = grabLabel(mylabel);
  BOOST_CHECK_EQUAL(std::string(mylabel), result);
}

BOOST_AUTO_TEST_CASE ( InputTag_comparison )
{
  art::InputTag a1("alabel:aname:aprocess"),
    a2("alabel:aname:aprocess"),
    a3(a1),
    b1("alabel:aname:anotherprocess"),
    b2("alabel:anothername:aprocess"),
    b3("anotherlabel:aname:aprocess");
  BOOST_CHECK_EQUAL(a1, a2);
  BOOST_CHECK_EQUAL(a1, a3);
  BOOST_CHECK_EQUAL(a2, a3);
  BOOST_CHECK_EQUAL(a3, a1);
  BOOST_CHECK_NE(a1, b1);
  BOOST_CHECK_NE(a1, b2);
  BOOST_CHECK_NE(a1, b3);
  BOOST_CHECK_NE(b1, b2);
  BOOST_CHECK_NE(b1, b3);
  BOOST_CHECK_NE(b2, b3);
}

BOOST_AUTO_TEST_SUITE_END()

