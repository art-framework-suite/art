#include "art/Framework/Core/RootDictionaryManager.h"

using namespace art;

#define BOOST_TEST_MODULE ( RootDictionaryManager Test )
#include "boost/test/auto_unit_test.hpp"

#include "cpp0x/algorithm"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include <iostream>
#include <iterator>

using cet::LibraryManager;

struct RootDictionaryManagerTestFixture {

   RootDictionaryManagerTestFixture();
   ~RootDictionaryManagerTestFixture() {}

   RootDictionaryManager rdm;
   RootDictionaryManager const &rdm_ref;

};

RootDictionaryManagerTestFixture::RootDictionaryManagerTestFixture()
   :
   rdm(),
   rdm_ref(rdm)
{
}

BOOST_FIXTURE_TEST_SUITE ( RootDictionaryManagerTests, RootDictionaryManagerTestFixture )

BOOST_AUTO_TEST_CASE ( DictLoaded )
{
   LibraryManager dlm("dict");
   std::vector<std::string> lib_list;
   dlm.getLoadableLibraries(lib_list);
   BOOST_REQUIRE(lib_list.size() > 1);
   BOOST_REQUIRE(rdm_ref.dictIsLoaded(*lib_list.begin()));
}

BOOST_AUTO_TEST_CASE ( DictNotLoaded )
{
   BOOST_REQUIRE(!rdm_ref.dictIsLoaded("UnknownLibrary"));
}

BOOST_AUTO_TEST_CASE ( DictLoadable )
{
   LibraryManager dlm("dict");
   std::vector<std::string> lib_list;
   dlm.getLoadableLibraries(lib_list);
   BOOST_REQUIRE(lib_list.size() > 1);
   BOOST_REQUIRE(rdm_ref.dictIsLoadable(*lib_list.begin()));
}

BOOST_AUTO_TEST_CASE ( DictNotLoadable )
{
   BOOST_REQUIRE(!rdm_ref.dictIsLoadable("UnknownLibrary"));
}

BOOST_AUTO_TEST_CASE ( dumpReflexDictionaryInfoOne )
{
   LibraryManager dlm("dict");
   std::vector<std::string> lib_list;
   dlm.getLoadableLibraries(lib_list);
   BOOST_REQUIRE(lib_list.size() > 1);
   std::ostringstream out;
   rdm_ref.dumpReflexDictionaryInfo(out, *lib_list.begin());
   BOOST_REQUIRE(out.str().size() > 5);
}

BOOST_AUTO_TEST_CASE ( dumpReflexDictionaryInfoAll )
{
   std::ostringstream out;
   rdm_ref.dumpReflexDictionaryInfo(out);
   BOOST_REQUIRE(out.str().size() > 5);
   std::cerr << out.str();
}

BOOST_AUTO_TEST_SUITE_END()
