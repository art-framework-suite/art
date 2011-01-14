extern "C" { int artapp(int argc, char* argv[]); }

#define BOOST_TEST_MODULE ( artapp test )
#include "boost/test/auto_unit_test.hpp"

BOOST_AUTO_TEST_SUITE ( artappTests )

BOOST_AUTO_TEST_CASE ( test_view_01 ) {
   char const* strings[] = { "artapp_t", "--config", "test_view_01.fcl" };
   BOOST_CHECK( artapp(3, const_cast<char**>(strings) ) == 0 );
   BOOST_CHECK( system("grep -e 'Event  Summary' warnings.log >/dev/null 2>&1") == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
