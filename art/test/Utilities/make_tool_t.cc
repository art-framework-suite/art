#define BOOST_TEST_MODULE ( make_tool_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/Utilities/make_tool.h"
#include "art/test/Utilities/ClassTool.h"

#include <string>


using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(make_tool_t)

BOOST_AUTO_TEST_CASE(tool_class)
{
  fhicl::ParameterSet ps;
  ps.put("tool_type", "ClassTool"s);
  auto t1 = art::make_tool<arttest::ClassTool>(ps);
}

BOOST_AUTO_TEST_CASE(tool_function)
{
  fhicl::ParameterSet ps;
  ps.put("tool_type", "FunctionTool"s);
  auto t2 = art::make_tool<int(int)>(ps);
  BOOST_CHECK_EQUAL(t2(17), 17);
}

BOOST_AUTO_TEST_SUITE_END()
