#define BOOST_TEST_MODULE (TypeWithDict_t)
#include "boost/test/auto_unit_test.hpp"

#include "art/Persistency/Provenance/TypeWithDict.h"
#include "art/Utilities/uniform_type_name.h"

#include "RtypesCore.h"

#include <vector>

using art::TypeWithDict;

BOOST_AUTO_TEST_SUITE(TypeWithDict_t)

BOOST_AUTO_TEST_CASE(default_construct)
{
  TypeWithDict td;
  BOOST_REQUIRE(!td);
  BOOST_REQUIRE_EQUAL(td.category(), TypeWithDict::Category::NONE);
}

BOOST_AUTO_TEST_CASE(class_from_typeid)
{
  TypeWithDict td(typeid(std::vector<int>));
  BOOST_REQUIRE(td);
  BOOST_REQUIRE_EQUAL(td.category(), TypeWithDict::Category::CLASSTYPE);
  BOOST_REQUIRE_EQUAL(td.className(), std::string("std::vector<int>"));
}

BOOST_AUTO_TEST_CASE(class_from_name)
{
  TypeWithDict td("std::vector<int>");
  BOOST_REQUIRE(td);
  BOOST_REQUIRE_EQUAL(td.category(), TypeWithDict::Category::CLASSTYPE);
  BOOST_REQUIRE_EQUAL(td.className(), std::string("std::vector<int>"));
}

#define test_semibasic_type(type, tname, testid)                        \
  BOOST_AUTO_TEST_CASE(testid##_from_typeid)                            \
  {                                                                     \
    TypeWithDict td(typeid(type));                                      \
    BOOST_REQUIRE(td);                                                  \
    BOOST_REQUIRE_EQUAL(td.category(), TypeWithDict::Category::BASICTYPE); \
    BOOST_REQUIRE_EQUAL(td.className(), art::uniform_type_name(#tname)); \
  }                                                                     \
                                                                        \
  BOOST_AUTO_TEST_CASE(testid##_from_name)                              \
  {                                                                     \
    TypeWithDict td(#type);                                             \
    BOOST_REQUIRE(td);                                                  \
    BOOST_REQUIRE_EQUAL(td.category(), TypeWithDict::Category::BASICTYPE); \
    BOOST_REQUIRE_EQUAL(td.className(), art::uniform_type_name(#tname)); \
  }

#define test_basic_type(type, testid) test_semibasic_type(type,type,testid)

test_basic_type(char,c)
test_basic_type(short,s)
test_basic_type(int,i)
test_basic_type(long,l)
test_basic_type(float,f)
test_basic_type(double,d)
test_semibasic_type(Double32_t,double,d32)
test_basic_type(unsigned char,uc)
test_basic_type(unsigned short,us)
test_basic_type(unsigned int,ui)
test_basic_type(unsigned long,ul)
test_basic_type(long long,ll)
test_basic_type(unsigned long long,ull)

BOOST_AUTO_TEST_CASE(t04)
{
}

BOOST_AUTO_TEST_CASE(t05)
{
}

BOOST_AUTO_TEST_CASE(t06)
{
}

BOOST_AUTO_TEST_CASE(t07)
{
}

BOOST_AUTO_TEST_CASE(t08)
{
}

BOOST_AUTO_TEST_CASE(t09)
{
}

BOOST_AUTO_TEST_CASE(t10)
{
}

BOOST_AUTO_TEST_CASE(t11)
{
}

BOOST_AUTO_TEST_CASE(t12)
{
}

BOOST_AUTO_TEST_CASE(t13)
{
}

BOOST_AUTO_TEST_CASE(t14)
{
}

BOOST_AUTO_TEST_CASE(t15)
{
}

BOOST_AUTO_TEST_CASE(t16)
{
}

BOOST_AUTO_TEST_CASE(t17)
{
}

BOOST_AUTO_TEST_CASE(t18)
{
}

BOOST_AUTO_TEST_CASE(t19)
{
}

BOOST_AUTO_TEST_CASE(t20)
{
}

BOOST_AUTO_TEST_CASE(t21)
{
}

BOOST_AUTO_TEST_SUITE_END()
