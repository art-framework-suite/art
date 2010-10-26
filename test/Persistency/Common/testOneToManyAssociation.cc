//
#include <cppunit/extensions/HelperMacros.h>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "art/Persistency/Common/AssociationMap.h"

class testOneToManyAssociation : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testOneToManyAssociation);
  CPPUNIT_TEST(checkAll);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}
  void tearDown() {}
  void checkAll();
  void dummy();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testOneToManyAssociation);

void testOneToManyAssociation::checkAll() {
  typedef std::vector<int> CKey;
  typedef std::vector<double> CVal;
  typedef art::AssociationMap<art::OneToMany<CKey, CVal, unsigned char> > Assoc;
  Assoc v;
  CPPUNIT_ASSERT(v.empty());
  CPPUNIT_ASSERT(v.size() == 0);
}

// just check that some stuff compiles
void testOneToManyAssociation::dummy() {
  typedef std::vector<int> CKey;
  typedef std::vector<double> CVal;
  {
    typedef art::AssociationMap<art::OneToMany<CKey, CVal, unsigned char> > Assoc;
    Assoc v;
    v.insert(art::Ref<CKey>(), art::Ref<CVal>());
    v.insert(Assoc::value_type(art::Ref<CKey>(), art::RefVector<CVal>()));
    Assoc::const_iterator b = v.begin(), e = v.end();
    ++b; ++e;
    Assoc::const_iterator f = v.find(art::Ref<CKey>());
    v.numberOfAssociations(art::Ref<CKey>());
    const art::RefVector<CVal> & x = v[art::Ref<CKey>()]; x.size();
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
  {
    typedef art::AssociationMap<art::OneToManyWithQuality<CKey, CVal, double, unsigned char> > Assoc;
    Assoc v;
    v.insert(art::Ref<CKey>(), std::make_pair(art::Ref<CVal>(), 3.14));
    Assoc::const_iterator b = v.begin(), e = v.end();
    ++b; ++e;
    Assoc::const_iterator f = v.find(art::Ref<CKey>());
    v.numberOfAssociations(art::Ref<CKey>());
    const std::vector<std::pair<art::Ref<CVal>, double> > & x = v[art::Ref<CKey>()]; x.size();
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
}
