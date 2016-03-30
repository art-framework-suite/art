#include "art/Utilities/HRRealTime.h"

#include "test/CppUnit_testdriver.icpp"
#include <cppunit/extensions/HelperMacros.h>

#include <cmath>
#include <ctime>
#include <iostream>
#include <typeinfo>

namespace {

  double gcrap=0;
  void waiste() {
    for (double i=1;i<100000;i++)
      gcrap+=std::log(std::sqrt(i));
  }
}

// FIXME
// I would have preferred check by features....
class TestTimers : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestTimers);
  CPPUNIT_TEST(check_stdclock);
  CPPUNIT_TEST(check_RealTime);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() override {}
  void tearDown() override {}
  void check_stdclock();
  void check_RealTime();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTimers);


template<typename S>
void checkTime(S source, bool hr) {

  typedef art::HRTimeDiffType T;

  T i = source();

  CPPUNIT_ASSERT(!( i<0 ));

  // source()-source()  it seems that this may be negative...

  waiste();

  T a = source();
  T b = source();
  CPPUNIT_ASSERT(!( (a-i)<0 ));
  CPPUNIT_ASSERT(!( (b-a)<0 ));
  if (hr) CPPUNIT_ASSERT(a>i); // not obvious if low resolution

  waiste();

  T c = source();
  double d = double(source()-c);
  CPPUNIT_ASSERT(!(d<0));


  T e = source();
  CPPUNIT_ASSERT(!( (c-i)<(b-i) ));
  CPPUNIT_ASSERT(!( (e-i)<(c-i) ));
  if (hr) CPPUNIT_ASSERT( (e-i)>(b-i) ); // not obvious if low resolution...

}

#define CHECKTIME(S,HR) \
  std::cout << "checking source " << #S << std::endl; \
  checkTime(&S,HR)


void TestTimers::check_stdclock() {
  CHECKTIME(std::clock,false);
}

void TestTimers::check_RealTime() {
  CHECKTIME(art::hrRealTime,true);
}
