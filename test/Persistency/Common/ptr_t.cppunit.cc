/*
 *  ptr_t.cppunit.cc
 *  CMSSW
 *
 *  Created by Chris Jones on 02/11/05.
 *
 */

#include <cppunit/extensions/HelperMacros.h>
#include <vector>
#include <set>
#include <list>
#include <deque>
#include "art/Persistency/Common/Ptr.h"

#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/Wrapper.h"

#include <iostream>

#include "test/Persistency/Common/IntValues.h"
#include <typeinfo>

using namespace art;
using namespace test_with_reflex;

class testPtr: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testPtr);

  CPPUNIT_TEST(constructTest);
  CPPUNIT_TEST(comparisonTest);
  CPPUNIT_TEST(getTest);

  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}
  void tearDown() {}

  void constructTest();
  void comparisonTest();
  void getTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testPtr);

namespace {
  struct Dummy {
    Dummy() {}
    virtual ~Dummy() {}
    bool operator==(Dummy const & iRHS) const {return this == &iRHS;}
    bool operator<(Dummy const & iRHS) const { return this->address() < iRHS.address(); }
    void const * address() const {return this;}
  };

  typedef std::vector<Dummy> DummyCollection;

  typedef std::set<Dummy> DummySet;
  typedef std::list<Dummy> DummyList;
  typedef std::deque<Dummy> DummyDeque;

  struct Dummy2 : public Dummy {
  };

  typedef std::vector<Dummy2> DummyCollection2;
}

void testPtr::constructTest()
{
  Ptr<Dummy> nulled;
  CPPUNIT_ASSERT(!nulled);
  CPPUNIT_ASSERT(nulled.isNull());
  CPPUNIT_ASSERT(!nulled.isNonnull());
  CPPUNIT_ASSERT(!nulled.isAvailable());
  Ptr<Dummy> nulledP;
  CPPUNIT_ASSERT(!nulledP);
  CPPUNIT_ASSERT(nulledP.isNull());
  CPPUNIT_ASSERT(!nulledP.isNonnull());
  CPPUNIT_ASSERT(!nulled.isAvailable());
  ProductID const pid(1, 1);
  {
    unsigned int const key = 2;
    Dummy const dummy;
    DummyCollection dummyContainer;
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    Ptr<Dummy> dummyPtr(pid, &dummyContainer, key);
    CPPUNIT_ASSERT(dummyPtr.isAvailable());
    CPPUNIT_ASSERT(dummyPtr.id() == pid);
    //CPPUNIT_ASSERT(dummyPtrProd.id() == pid);
    CPPUNIT_ASSERT(dummyPtr.key() == key);
    CPPUNIT_ASSERT(dummyPtr.get() == &dummyContainer[key]);
    CPPUNIT_ASSERT(&(*dummyPtr) == &dummyContainer[key]);
    CPPUNIT_ASSERT((dummyPtr.operator->()) == &dummyContainer[key]);
    CPPUNIT_ASSERT(dummyPtr->address() == dummyContainer[key].address());
  }
  {
    unsigned int const key = 2;
    Dummy const dummy;
    DummySet dummyContainer;
    dummyContainer.insert(dummy);
    dummyContainer.insert(dummy);
    dummyContainer.insert(dummy);
    Ptr<Dummy> dummyPtr(pid, &dummyContainer, key);
    CPPUNIT_ASSERT(dummyPtr.isAvailable());
    CPPUNIT_ASSERT(dummyPtr.id() == pid);
    //CPPUNIT_ASSERT(dummyPtrProd.id() == pid);
    CPPUNIT_ASSERT(dummyPtr.key() == key);
    DummySet::const_iterator it = dummyContainer.begin();
    std::advance(it, key);
    CPPUNIT_ASSERT(dummyPtr.get() == it->address());
    CPPUNIT_ASSERT(&(*dummyPtr) == it->address());
    CPPUNIT_ASSERT((dummyPtr.operator->()) == it->address());
    CPPUNIT_ASSERT(dummyPtr->address() == it->address());
  }
  {
    unsigned int const key = 2;
    Dummy const dummy;
    DummyList dummyContainer;
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    Ptr<Dummy> dummyPtr(pid, &dummyContainer, key);
    CPPUNIT_ASSERT(dummyPtr.isAvailable());
    CPPUNIT_ASSERT(dummyPtr.id() == pid);
    //CPPUNIT_ASSERT(dummyPtrProd.id() == pid);
    CPPUNIT_ASSERT(dummyPtr.key() == key);
    DummyList::const_iterator it = dummyContainer.begin();
    std::advance(it, key);
    CPPUNIT_ASSERT(dummyPtr.get() == it->address());
    CPPUNIT_ASSERT(&(*dummyPtr) == it->address());
    CPPUNIT_ASSERT((dummyPtr.operator->()) == it->address());
    CPPUNIT_ASSERT(dummyPtr->address() == it->address());
  }
  {
    unsigned int const key = 2;
    Dummy const dummy;
    DummyDeque dummyContainer;
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    Ptr<Dummy> dummyPtr(pid, &dummyContainer, key);
    CPPUNIT_ASSERT(dummyPtr.isAvailable());
    CPPUNIT_ASSERT(dummyPtr.id() == pid);
    //CPPUNIT_ASSERT(dummyPtrProd.id() == pid);
    CPPUNIT_ASSERT(dummyPtr.key() == key);
    DummyDeque::const_iterator it = dummyContainer.begin();
    std::advance(it, key);
    CPPUNIT_ASSERT(dummyPtr.get() == it->address());
    CPPUNIT_ASSERT(&(*dummyPtr) == it->address());
    CPPUNIT_ASSERT((dummyPtr.operator->()) == it->address());
    CPPUNIT_ASSERT(dummyPtr->address() == it->address());
  }
  {
    unsigned int const key = 2;
    Dummy2 const dummy;
    DummyCollection2 dummyContainer;
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    dummyContainer.push_back(dummy);
    Ptr<Dummy> dummyPtr(pid, &dummyContainer, key);
    Ptr<Dummy2> dummyPtr2(dummyPtr);
    CPPUNIT_ASSERT(dummyPtr.isAvailable());
    CPPUNIT_ASSERT(dummyPtr.id() == pid);
    //CPPUNIT_ASSERT(dummyPtrProd.id() == pid);
    CPPUNIT_ASSERT(dummyPtr.key() == key);
    CPPUNIT_ASSERT(dummyPtr.get() == &dummyContainer[key]);
    CPPUNIT_ASSERT(&(*dummyPtr) == &dummyContainer[key]);
    CPPUNIT_ASSERT((dummyPtr.operator->()) == &dummyContainer[key]);
    CPPUNIT_ASSERT(dummyPtr->address() == dummyContainer[key].address());
    CPPUNIT_ASSERT(dummyPtr2.isAvailable());
    CPPUNIT_ASSERT(dummyPtr2.id() == pid);
    CPPUNIT_ASSERT(dummyPtr2.key() == key);
    CPPUNIT_ASSERT(dummyPtr2.get() == &dummyContainer[key]);
    CPPUNIT_ASSERT(&(*dummyPtr2) == &dummyContainer[key]);
    CPPUNIT_ASSERT((dummyPtr2.operator->()) == &dummyContainer[key]);
    CPPUNIT_ASSERT(dummyPtr2->address() == dummyContainer[key].address());
    Ptr<Dummy2> dummy2Ptr(pid, &dummyContainer, key);
    Ptr<Dummy> copyPtr(dummy2Ptr);
    CPPUNIT_ASSERT(dummy2Ptr.key() == copyPtr.key());
    CPPUNIT_ASSERT(dummy2Ptr.get() == static_cast<const Dummy *>(dummy2Ptr.get()));
  }
}

void testPtr::comparisonTest()
{
  {
    ProductID const pid(1, 1);
    unsigned int const key = 2;
    Dummy const dummy;
    DummyCollection dummyContainer;
    dummyContainer.push_back(dummy);
    Ptr<Dummy> dummyPtr1(pid, &dummyContainer, key);
    Ptr<Dummy> dummyPtr2(pid, &dummyContainer, key);
    CPPUNIT_ASSERT(dummyPtr1 == dummyPtr2);
    CPPUNIT_ASSERT(!(dummyPtr1 != dummyPtr2));
    CPPUNIT_ASSERT(!(dummyPtr1 < dummyPtr2));
    CPPUNIT_ASSERT(!(dummyPtr2 < dummyPtr1));
    CPPUNIT_ASSERT(!(dummyPtr1 < dummyPtr1));
    CPPUNIT_ASSERT(!(dummyPtr2 < dummyPtr2));
    Ptr<Dummy> dummyPtrNewKey(pid, &dummyContainer, key + 1);
    CPPUNIT_ASSERT(!(dummyPtr1 == dummyPtrNewKey));
    CPPUNIT_ASSERT(dummyPtr1 != dummyPtrNewKey);
    CPPUNIT_ASSERT(dummyPtr1 < dummyPtrNewKey);
    CPPUNIT_ASSERT(!(dummyPtrNewKey < dummyPtr1));
    ProductID const pidOther(1, 4);
    Ptr<Dummy> dummyPtrNewPID(pidOther, &dummyContainer, key);
    CPPUNIT_ASSERT(!(dummyPtr1 == dummyPtrNewPID));
    CPPUNIT_ASSERT(dummyPtr1 != dummyPtrNewPID);
  }
}


namespace {
  struct TestGetter : public art::EDProductGetter {
    EDProduct const * hold_;
    EDProduct const * getIt(ProductID const &) const {
      return hold_;
    }

    TestGetter() : hold_(0) {}
  };
}


void testPtr::getTest()
{
  typedef std::vector<IntValue> IntCollection;
  std::auto_ptr<IntCollection> ptr(new IntCollection);
  ptr->push_back(0);
  ptr->push_back(1);
  art::Wrapper<IntCollection> wrapper(ptr);
  TestGetter tester;
  tester.hold_ = &wrapper;
  ProductID const pid(1, 1);
  IntCollection const * wptr = dynamic_cast<IntCollection const *>(wrapper.product());
  Ptr<IntValue> ref0(pid, 0, &tester);
  CPPUNIT_ASSERT(!ref0.hasCache());
  Ptr<IntValue> ref1(pid, 1, &tester);
  Ptr<IntValue> ref2(pid, 1, &tester);
  CPPUNIT_ASSERT(ref0.isAvailable());
  CPPUNIT_ASSERT(0 == ref0->value_);
  CPPUNIT_ASSERT(ref0.hasCache());
  CPPUNIT_ASSERT(1 == ref1->value_);
  CPPUNIT_ASSERT(1 == ref2->value_);
  CPPUNIT_ASSERT(1 == (*ref1).value_);
  {
    typedef std::vector<IntValue2> SDCollection;
    std::auto_ptr<SDCollection> ptr(new SDCollection);
    ptr->push_back(IntValue2(0));
    ptr->back().value_ = 0;
    ptr->push_back(IntValue2(1));
    ptr->back().value_ = 1;
    art::Wrapper<SDCollection> wrapper(ptr);
    TestGetter tester;
    tester.hold_ = &wrapper;
    ProductID const pid(1, 1);
    SDCollection const * wptr = dynamic_cast<SDCollection const *>(wrapper.product());
    Ptr<IntValue> ref0(pid, 0, &tester);
    CPPUNIT_ASSERT(!ref0.hasCache());
    Ptr<IntValue> ref1(pid, 1, &tester);
    Ptr<IntValue> ref2(pid, 1, &tester);
    CPPUNIT_ASSERT(0 == ref0->value_);
    CPPUNIT_ASSERT(ref0.hasCache());
    CPPUNIT_ASSERT(1 == ref1->value_);
    CPPUNIT_ASSERT(1 == ref2->value_);
    CPPUNIT_ASSERT(1 == (*ref1).value_);
  }
  {
    TestGetter tester;
    tester.hold_ = 0;
    ProductID const pid(1, 1);
    Ptr<IntValue> ref0(pid, 0, &tester);
    CPPUNIT_ASSERT_THROW((*ref0), cet::exception);
    CPPUNIT_ASSERT_THROW((ref0.operator->()), cet::exception);
  }
}
