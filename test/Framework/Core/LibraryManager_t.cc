#include "art/Framework/Core/LibraryManager.h"

using namespace art;

#define BOOST_TEST_MODULE ( LibraryManager Test )
#include "boost/test/auto_unit_test.hpp"

#include "cpp0x/algorithm"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include <iostream>
#include <iterator>

struct LibraryManagerTestFixture {

   LibraryManagerTestFixture();
   ~LibraryManagerTestFixture();

   LibraryManager *lm;
   LibraryManager const &lm_ref;

};

LibraryManagerTestFixture::LibraryManagerTestFixture()
   :
   lm(new LibraryManager("plugin")),
   lm_ref(*lm)
{
//    BOOST_TEST_MESSAGE( "Create LibraryManager(\"plugin\")." );
}
   
LibraryManagerTestFixture::~LibraryManagerTestFixture()
{
//    BOOST_TEST_MESSAGE( "Clean up." );
   delete lm;
};

BOOST_FIXTURE_TEST_SUITE ( LibraryManagerTests, LibraryManagerTestFixture )

BOOST_AUTO_TEST_CASE ( libSpecsVector )
{
   std::vector<std::string> lib_list;
   BOOST_REQUIRE(lm_ref.getValidLibspecs(lib_list) > 0);
//    BOOST_TEST_MESSAGE( "List of valid libspec values:" );
//    cet::copy_all(lib_list, std::ostream_iterator<std::string>(std::cerr, "\n"));
}

BOOST_AUTO_TEST_CASE (libSpecsIter)
{
   std::vector<std::string> lib_list;
   BOOST_REQUIRE(lm_ref.getValidLibspecs(std::back_inserter(lib_list)) > 0);
}

BOOST_AUTO_TEST_CASE ( libListVector )
{
   std::vector<std::string> lib_list;
   BOOST_REQUIRE(lm_ref.getLoadableLibraries(lib_list) > 0);
//    BOOST_TEST_MESSAGE( "List of loadable libraries:" );
//    cet::copy_all(lib_list, std::ostream_iterator<std::string>(std::cerr, "\n"));
}

BOOST_AUTO_TEST_CASE ( libListIter )
{
   std::vector<std::string> lib_list;
   BOOST_REQUIRE(lm_ref.getLoadableLibraries(std::back_inserter(lib_list)) > 0);
}

BOOST_AUTO_TEST_CASE ( getSymbolLong )
{
   BOOST_REQUIRE( lm_ref.getSymbol("art/Framework/IO/Output/PoolOutputModule",
                                   "_init") != nullptr );
}

BOOST_AUTO_TEST_CASE ( getSymbolShort )
{
   BOOST_REQUIRE( lm_ref.getSymbol("PoolOutputModule",
                                   "_init") != nullptr );
}

BOOST_AUTO_TEST_CASE ( getSymbolPathPrecedence )
{
   BOOST_REQUIRE_NO_THROW( lm_ref.getSymbol ( "1/1/1", "_init") );
}

BOOST_AUTO_TEST_CASE ( getSymbolAmbiguity )
{
   BOOST_REQUIRE_THROW( lm_ref.getSymbol ( "3", "_init") == nullptr, cet::exception );
}

BOOST_AUTO_TEST_CASE ( getSymbolNoAmbiguity )
{
   BOOST_REQUIRE_NO_THROW( lm_ref.getSymbol ( "2/1/3", "_init") == nullptr );
}

BOOST_AUTO_TEST_CASE ( loadAllLibraries )
{
   BOOST_REQUIRE_NO_THROW(lm_ref.loadAllLibraries());
}

BOOST_AUTO_TEST_SUITE_END()

