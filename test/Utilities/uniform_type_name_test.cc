#define BOOST_TEST_MODULE (uniform_type_name_test)
#include "boost/test/auto_unit_test.hpp"

#include "canvas/Utilities/uniform_type_name.h"

#include <map>
#include <string>
#include <vector>

using art::uniform_type_name;
using std::string;

template <unsigned int, typename>
struct MyUIntTemplate_t {
};

template <unsigned long, typename>
struct MyULongTemplate_t {
};

namespace {
  void testit(std::type_info const & tid,
              std::string const ref)
  {
    BOOST_CHECK_EQUAL(uniform_type_name(tid), ref);
  }
}

BOOST_AUTO_TEST_SUITE(uniform_type_name_test)

BOOST_AUTO_TEST_CASE(allocator_removal)
{
  testit(typeid(std::vector<std::string>),
         "std::vector<std::string>");
}

BOOST_AUTO_TEST_CASE(comparator_removal)
{
  testit(typeid(std::map<std::string, std::string>),
         "std::map<std::string,std::string>");
}

BOOST_AUTO_TEST_CASE(int_type_names)
{
  testit(typeid(unsigned long long), "ULong64_t");
  testit(typeid(long long), "Long64_t");
  testit(typeid(MyUIntTemplate_t<3u, std::string>),
         "MyUIntTemplate_t<3,std::string>");
  testit(typeid(MyULongTemplate_t<4ul, std::string>),
         "MyULongTemplate_t<4,std::string>");
}

BOOST_AUTO_TEST_SUITE_END()
