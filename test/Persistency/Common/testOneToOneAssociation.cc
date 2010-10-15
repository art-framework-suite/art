//
#include <cppunit/extensions/HelperMacros.h>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "art/Persistency/Common/AssociationMap.h"

class testOneToOneAssociation : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testOneToOneAssociation);
  CPPUNIT_TEST(checkAll);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}
  void tearDown() {}
  void checkAll();
  void dummy();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testOneToOneAssociation);

void testOneToOneAssociation::checkAll() {
  typedef std::vector<int> CKey;
  typedef std::vector<double> CVal;
  typedef art::AssociationMap<art::OneToOne<CKey, CVal, unsigned char> > Assoc;
  Assoc v;
  CPPUNIT_ASSERT(v.empty());
  CPPUNIT_ASSERT(v.size() == 0);
}

// just check that some stuff compiles
void  testOneToOneAssociation::dummy() {
  typedef std::vector<int> CKey;
  typedef std::vector<double> CVal;
  typedef art::AssociationMap<art::OneToOne<CKey, CVal, unsigned char> > Assoc;
  Assoc v;
  v.insert(art::Ref<CKey>(), art::Ref<CVal>());
  v.insert(Assoc::value_type(art::Ref<CKey>(), art::Ref<CVal>()));
  Assoc::const_iterator b = v.begin(), e = v.end();
  ++b; ++e;
  Assoc::const_iterator f = v.find(art::Ref<CKey>());
  v.numberOfAssociations(art::Ref<CKey>());
  const art::Ref<CVal> & x = v[art::Ref<CKey>()]; x.id();
  ++f;
  int n = v.numberOfAssociations(art::Ref<CKey>());
  ++n;
  art::Ref<Assoc> r;
  v[art::Ref<CKey>()];
  v.erase(art::Ref<CKey>());
  v.clear();
  CPPUNIT_ASSERT(v.size() == 0);
  v.post_insert();
}
