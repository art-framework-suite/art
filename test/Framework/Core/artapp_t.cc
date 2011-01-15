extern "C" { int artapp(int argc, char* argv[]); }

#define BOOST_TEST_MODULE ( artapp test )
#include "boost/test/auto_unit_test.hpp"

#include "cetlib/exception.h"

BOOST_AUTO_TEST_SUITE ( artappTests )

BOOST_AUTO_TEST_CASE ( NoConfig ) {
   char const* strings[] = { "artapp_t" };
   BOOST_CHECK( artapp(1, const_cast<char**>(strings) ) == 7001 );
}

BOOST_AUTO_TEST_CASE ( testHelp ) {
   char const* strings[] = { "artapp_t", "--help" };
   BOOST_CHECK( artapp(2, const_cast<char**>(strings) ) == 1 );
}

BOOST_AUTO_TEST_CASE ( testBadConfigOption ) {
   char const* strings[] = { "artapp_t", "--config" };
   BOOST_CHECK( artapp(2, const_cast<char**>(strings) ) == 7000 );
}

BOOST_AUTO_TEST_CASE ( testEmptyConfig ) {
   char const* strings[] = { "artapp_t", "--config", "empty_config.fcl" };
   BOOST_CHECK( artapp(3, const_cast<char**>(strings) ) == 0 );
   BOOST_CHECK( system("grep -e '^%MSG-i MF_INIT_OK:' warnings.log >/dev/null 2>&1") == 0 );
}

BOOST_AUTO_TEST_CASE ( testSumaryOnlyConfig ) {
   char const* strings[] = { "artapp_t", "--config", "summary_only_config.fcl" };
   BOOST_CHECK( artapp(3, const_cast<char**>(strings) ) == 0 );
   BOOST_CHECK( system("grep -e 'Event  Summary' warnings.log >/dev/null 2>&1") == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
