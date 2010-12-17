#include "art/Framework/Core/LibraryManager.h"

using namespace art;

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ( LibraryManager Test )
#include "boost/test/auto_unit_test.hpp"

#include "cpp0x/algorithm"

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
   BOOST_TEST_MESSAGE( "Create LibraryManager(\"plugin\")." );
}
   
LibraryManagerTestFixture::~LibraryManagerTestFixture()
{
   BOOST_TEST_MESSAGE( "Clean up." );
   delete lm;
};

BOOST_FIXTURE_TEST_SUITE ( LibraryManagertTests, LibraryManagerTestFixture )

BOOST_AUTO_TEST_CASE ( libSpecsVector )
{
   std::vector<std::string> lib_list;
   BOOST_REQUIRE(lm_ref.getValidLibspecs(lib_list) > 0);
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
}

BOOST_AUTO_TEST_CASE ( libListIter )
{
   std::vector<std::string> lib_list;
   BOOST_REQUIRE(lm_ref.getLoadableLibraries(std::back_inserter(lib_list)) > 0);
}

BOOST_AUTO_TEST_CASE ( loadAllLibraries )
{
   BOOST_REQUIRE_NO_THROW(lm_ref.loadAllLibraries());
}

BOOST_AUTO_TEST_CASE ( getSymbol )
{
   std::vector<std::string> lib_list;
   lm_ref.getValidLibspecs(lib_list);
   BOOST_REQUIRE( lm_ref.getSymbol(*lib_list.begin(),
                                   "_init") != nullptr );
}

BOOST_AUTO_TEST_SUITE_END()

