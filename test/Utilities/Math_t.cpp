#include <iostream>
#include <limits>

#include "art/Utilities/Math.h"
#include "art/Utilities/HRRealTime.h"

#include "test/CppUnit_testdriver.icpp"
#include <cppunit/extensions/HelperMacros.h>


class TestMath : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestMath);
  CPPUNIT_TEST(test_isnan);
  CPPUNIT_TEST(timing);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown() {}
  void test_isnan();
  void timing();
};

template <class FP>
void
test_fp_type()
{
  CPPUNIT_ASSERT(std::numeric_limits<FP>::is_specialized);
  CPPUNIT_ASSERT(std::numeric_limits<FP>::has_quiet_NaN);
  FP nan = std::numeric_limits<FP>::quiet_NaN();
  CPPUNIT_ASSERT(!art::isnan(static_cast<FP>(1.0)));
  CPPUNIT_ASSERT(!art::isnan(static_cast<FP>(-1.0)));
  CPPUNIT_ASSERT(!art::isnan(static_cast<FP>(0.0)));
  CPPUNIT_ASSERT(!art::isnan(static_cast<FP>(-0.0)));
  CPPUNIT_ASSERT(!art::isnan(std::numeric_limits<FP>::infinity()));
  CPPUNIT_ASSERT(!art::isnan(-std::numeric_limits<FP>::infinity()));

  CPPUNIT_ASSERT(art::isnan(nan));
  CPPUNIT_ASSERT(art::isnan(std::numeric_limits<FP>::signaling_NaN()));
}



CPPUNIT_TEST_SUITE_REGISTRATION(TestMath);

void TestMath::test_isnan()
{
  test_fp_type<float>();
  test_fp_type<double>();
  test_fp_type<long double>();

  CPPUNIT_ASSERT(art::isnan(std::numeric_limits<float>::quiet_NaN()));
  CPPUNIT_ASSERT(art::isnan(std::numeric_limits<double>::quiet_NaN()));
  CPPUNIT_ASSERT(art::isnan(std::numeric_limits<long double>::quiet_NaN()));
}

template <class FP>
void
time_fp_type()
{
  volatile FP values[] = { FP(1.0), FP(1.0/0.0), FP(-2.5), FP(1.0/3.0), FP(0.0/0.0) };
  unsigned long sum = 0;

  art::HRTimeType start = art::hrRealTime();
  for (int i = 0; i < 1000*1000; ++i)
    for (int j = 0; j < 5; ++j)
      sum += (std::isnan(values[j]) ? 0 : 1);
  art::HRTimeType stop = art::hrRealTime();
  std::cout << "std::isnan time:         " << (stop - start) << std::endl;

  sum = 0;
  start = art::hrRealTime();
  for (int i = 0; i < 1000*1000; ++i)
    for (int j = 0; j < 5; ++j)
      sum += (art::detail::isnan(values[j]) ? 0 : 1);
  stop = art::hrRealTime();
  stop = art::hrRealTime();
  std::cout << "art::detail::isnan time: " << (stop - start) << std::endl;

  sum = 0;
  start = art::hrRealTime();
  for (int i = 0; i < 1000*1000; ++i)
    for (int j = 0; j < 5; ++j)
      sum += (art::isnan(values[j]) ? 0 : 1);
  stop = art::hrRealTime();
  std::cout << "art::isnan time:         " << (stop - start) << std::endl;

  sum = 0;
  start = art::hrRealTime();
  for (int i = 0; i < 1000*1000; ++i)
    for (int j = 0; j < 5; ++j)
      sum += (art::equal_isnan(values[j]) ? 0 : 1);
  stop = art::hrRealTime();
  std::cout << "art::equal_isnan time:   " << (stop - start) << std::endl;
}


void TestMath::timing()
{
  std::cout << "\n\ntiming floats\n";
  time_fp_type<float>();
  std::cout << "\ntiming doubles\n";
  time_fp_type<double>();
  std::cout << "\ntiming long doubles\n";
  time_fp_type<long double>();
}
