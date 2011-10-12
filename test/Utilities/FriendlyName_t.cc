#define BOOST_TEST_MODULE ( FriendlyName_t )
#include "boost/test/auto_unit_test.hpp"

#include "art/Utilities/FriendlyName.h"

#include <map>
#include <string>

namespace {
  typedef std::map<std::string, std::string> fnmap_t;
}

struct FriendlyNameTestFixture {
  FriendlyNameTestFixture();

  fnmap_t nameMap;
};

FriendlyNameTestFixture::FriendlyNameTestFixture()
  :
  nameMap()
{
  nameMap.insert(std::make_pair("Foo", "Foo"));
  nameMap.insert(std::make_pair("bar::Foo", "bar::Foo"));
  nameMap.insert(std::make_pair("std::vector<Foo>", "Foos"));
  nameMap.insert(std::make_pair("std::vector<bar::Foo>", "bar::Foos"));
  nameMap.insert(std::make_pair("V<A,B>", "ABV"));
  nameMap.insert(std::make_pair("art::Wrapper<MuonDigiCollection<CSCDetId,CSCALCTDigi> >", "CSCDetIdCSCALCTDigiMuonDigiCollection"));
  nameMap.insert(std::make_pair("A<B<C>,D<E> >", "CBEDA"));
  nameMap.insert(std::make_pair("A<B<C<D> > >", "DCBA"));
  nameMap.insert(std::make_pair("A<B<C,D>,E<F> >", "CDBFEA"));
  nameMap.insert(std::make_pair("Aa<Bb<Cc>,Dd<Ee> >", "CcBbEeDdAa"));
  nameMap.insert(std::make_pair("Aa<Bb<Cc<Dd> > >", "DdCcBbAa"));
  nameMap.insert(std::make_pair("Aa<Bb<Cc,Dd>,Ee<Ff> >", "CcDdBbFfEeAa"));
  nameMap.insert(std::make_pair("Aa<Bb<Cc,Dd>,Ee<Ff,Gg> >", "CcDdBbFfGgEeAa"));
  nameMap.insert(std::make_pair("cet::map_vector_key", "mvk"));
  nameMap.insert(std::make_pair("cet::map_vector<Foo>", "Foomv"));
  nameMap.insert(std::make_pair("art::Assns<Ll,Rr,Dd>", "LlRrDdart::Assns"));
  nameMap.insert(std::make_pair("art::Assns<Rr,Ll,Dd>", "LlRrDdart::Assns"));
}

BOOST_FIXTURE_TEST_SUITE ( FriendlyName_t, FriendlyNameTestFixture )

  BOOST_AUTO_TEST_CASE( FriendlyName_t )
{
  for (fnmap_t::const_iterator
         i = nameMap.begin(),
         e = nameMap.end();
       i != e;
       ++i) {
    BOOST_CHECK_EQUAL(art::friendlyname::friendlyName(i->first), i->second);
  }
}

BOOST_AUTO_TEST_SUITE_END()
