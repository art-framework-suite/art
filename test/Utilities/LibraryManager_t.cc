#include "art/Utilities/LibraryManager.h"

using namespace art;

#define BOOST_TEST_MODULE ( LibraryManager Test )
#include "boost/test/auto_unit_test.hpp"

#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "cpp0x/algorithm"

#include <iostream>
#include <iterator>

struct LibraryManagerTestFixture {

  LibraryManagerTestFixture();
  ~LibraryManagerTestFixture();

  LibraryManager lm;
  LibraryManager const & lm_ref;
};

LibraryManagerTestFixture::LibraryManagerTestFixture()
  :
  lm("module"),
  lm_ref(lm)
{
}

LibraryManagerTestFixture::~LibraryManagerTestFixture()
{
}

BOOST_FIXTURE_TEST_SUITE(LibraryManagerTests, LibraryManagerTestFixture)

BOOST_AUTO_TEST_CASE(libSpecsVector)
{
  std::vector<std::string> lib_list;
  BOOST_REQUIRE(lm_ref.getValidLibspecs(lib_list) > 0);
  //    BOOST_TEST_MESSAGE( "List of valid libspec values:" );
  //    cet::copy_all(lib_list, std::ostream_iterator<std::string>(std::cerr, "\n"));
}

BOOST_AUTO_TEST_CASE(libSpecsIter)
{
  std::vector<std::string> lib_list;
  BOOST_REQUIRE(lm_ref.getValidLibspecs(std::back_inserter(lib_list)) > 0);
}

BOOST_AUTO_TEST_CASE(libListVector)
{
  std::vector<std::string> lib_list;
  BOOST_REQUIRE(lm_ref.getLoadableLibraries(lib_list) > 0);
}

BOOST_AUTO_TEST_CASE(libListIter)
{
  std::vector<std::string> lib_list;
  BOOST_REQUIRE(lm_ref.getLoadableLibraries(std::back_inserter(lib_list)) > 0);
}

BOOST_AUTO_TEST_CASE(getSymbolLong)
{
  BOOST_REQUIRE(lm_ref.getSymbolByLibspec<void *>("art/Framework/Modules/BlockingPrescaler",
                "moduleType") != nullptr);
}

BOOST_AUTO_TEST_CASE(getSymbolShort)
{
  BOOST_REQUIRE(lm_ref.getSymbolByLibspec<void *>("BlockingPrescaler",
                "moduleType") != nullptr);
}

BOOST_AUTO_TEST_CASE(getSymbolPathPrecedence)
{
  BOOST_CHECK_NO_THROW(lm_ref.getSymbolByLibspec<void *> ("1/1/1", "moduleType"));
}

BOOST_AUTO_TEST_CASE(getSymbolAmbiguity)
{
  BOOST_CHECK_EXCEPTION(lm_ref.getSymbolByLibspec<void *> ("3", "moduleType"), \
                        art::Exception,                                 \
                        [](art::Exception const & e)                    \
                        {                                               \
                          return e.categoryCode() == art::errors::Configuration; \
                        });                                             \
}

BOOST_AUTO_TEST_CASE(getSymbolNoAmbiguity)
{
  BOOST_CHECK_NO_THROW(lm_ref.getSymbolByLibspec<void *> ("2/1/3", "moduleType"));
}

BOOST_AUTO_TEST_CASE(dictLoadable)
{
  std::vector<std::string> lib_list;
  lm_ref.getLoadableLibraries(lib_list);
  BOOST_REQUIRE(lm_ref.libraryIsLoadable(*lib_list.begin()));
}

BOOST_AUTO_TEST_CASE(dictLoadableButNotLoaded)
{
  std::vector<std::string> lib_list;
  lm_ref.getLoadableLibraries(lib_list);
  BOOST_REQUIRE(lm_ref.libraryIsLoadable(*lib_list.begin()));
  BOOST_REQUIRE(!lm_ref.libraryIsLoaded(*lib_list.begin()));
}

BOOST_AUTO_TEST_CASE(loadAllLibraries)
{
  BOOST_REQUIRE_NO_THROW(lm_ref.loadAllLibraries());
}

BOOST_AUTO_TEST_CASE(dictLoaded)
{
  std::vector<std::string> lib_list;
  lm_ref.getLoadableLibraries(lib_list);
  lm_ref.loadAllLibraries();
  BOOST_REQUIRE(lm_ref.libraryIsLoaded(*lib_list.begin()));
}

BOOST_AUTO_TEST_CASE(dictNotLoadable)
{
  std::vector<std::string> lib_list;
  lm_ref.getLoadableLibraries(lib_list);
  BOOST_REQUIRE(!lm_ref.libraryIsLoadable("UnknownLibrary"));
}

BOOST_AUTO_TEST_SUITE_END()
