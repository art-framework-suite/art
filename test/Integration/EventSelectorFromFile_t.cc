extern "C" { int artapp(int argc, char * argv[]); }

#define BOOST_TEST_MODULE ( EventSelector test using files )
#include "boost/test/auto_unit_test.hpp"

BOOST_AUTO_TEST_SUITE(EventSelectorFileTests)

BOOST_AUTO_TEST_CASE(DirectedWrite)
{
  char const * strings[] = { "artapp_t", "--config", "EventSelectorFromFile_w.fcl" };
  BOOST_CHECK(artapp(3, const_cast<char **>(strings)) == 0);
  BOOST_CHECK(system("grep -e 'Event  Summary' warnings.log >/dev/null 2>&1") == 0);
}

BOOST_AUTO_TEST_CASE(ReadFromFile1)
{
  char const * strings[] = { "artapp_t", "--config", "EventSelectorFromFile_r1.fcl" };
  BOOST_CHECK(artapp(3, const_cast<char **>(strings)) == 0);
  BOOST_CHECK(system("grep -e 'Event  Summary' warnings.log >/dev/null 2>&1") == 0);
}

BOOST_AUTO_TEST_SUITE_END()
