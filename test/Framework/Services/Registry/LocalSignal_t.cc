#define BOOST_TEST_MODULE (LocalSignal_t)
#include "boost/test/auto_unit_test.hpp"
#include "boost/test/output_test_stream.hpp"

#include "art/Framework/Services/Registry/LocalSignal.h"

#include <ostream>
#include <stdexcept>
#include <string>

namespace {
  typedef art::LocalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &, std::string const &> TestSignal2;
  typedef art::LocalSignal<art::detail::SignalResponseType::LIFO, void, std::ostream &, std::string const &> TestSignal2a;
  typedef art::LocalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &> TestSignal1;
  typedef art::LocalSignal<art::detail::SignalResponseType::FIFO, void> TestSignal0;
  template <uint16_t n>
  void
  testCallback(std::ostream & os, std::string const & text)
  {
    if (n > 0) {
      os << n << ": ";
    }
    os << text;
  }

  struct CallBackClass {
    void
    func(std::ostream & os, std::string const & text)
      {
        os << text;
      }
    void
    cfunc(std::ostream & os, std::string const & text) const
      {
        os << text;
      }
  };

  size_t const nSchedules = 3;
  art::ScheduleID const sID { 1 };
}

BOOST_AUTO_TEST_SUITE(LocalSignal_t)

BOOST_AUTO_TEST_CASE(TestSignal2_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text.\n" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watch(sID, testCallback<1>));
  BOOST_CHECK_NO_THROW(s.watch(sID, testCallback<2>));
  BOOST_CHECK_NO_THROW(s.watch(sID, testCallback<3>));
  std::string const cmp_text { std::string("1: ") + test_text +
      "2: " + test_text +
      "3: " + test_text };
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_equal(cmp_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2a_t)
{
  TestSignal2a s(nSchedules);
  std::string const test_text { "Test text.\n" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watch(sID, testCallback<1>));
  BOOST_CHECK_NO_THROW(s.watch(sID, testCallback<2>));
  BOOST_CHECK_NO_THROW(s.watch(sID, testCallback<3>));
  std::string const cmp_text { std::string("3: ") + test_text +
      "2: " + test_text +
      "1: " + test_text };
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_equal(cmp_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_func_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  CallBackClass cbc;
  BOOST_CHECK_NO_THROW(s.watch(sID, &CallBackClass::func, cbc));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_cfunc_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  CallBackClass const cbc;
  BOOST_CHECK_NO_THROW(s.watch(sID, &CallBackClass::cfunc, cbc));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal1_t)
{
  TestSignal1 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watch(sID,
                               [&test_text](auto& x){ testCallback<0>(x, test_text); }));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal0_t)
{
  TestSignal0 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  // 2012/02/13 CG This explicit reference to base clase below is
  // necessary because std::ref(os) fails due to a private typedef
  // output_test_stream::result_type (in Boost <=1.53.0 at least)
  // screwing up std::ref's attempt to determine whether
  // output_test_stream is a callable entity.
  std::ostringstream & osr __attribute__((unused))(os);
  BOOST_CHECK_NO_THROW(s.watch(sID,
                               [&osr, &test_text](){ testCallback<0>(osr, test_text); }));
  BOOST_CHECK_NO_THROW(s.invoke(sID));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_All_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll(testCallback<0>));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_clear_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll(testCallback<0>));
  BOOST_CHECK_NO_THROW(s.clear(sID));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_empty());
}

BOOST_AUTO_TEST_CASE(TestSignal2_clearAll_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll(testCallback<0>));
  BOOST_CHECK_NO_THROW(s.clearAll());
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_empty());
}

BOOST_AUTO_TEST_CASE(TestSignal1_All_t)
{
  TestSignal1 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll([&test_text](auto& x){
        testCallback<0>(x, test_text);
      }));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal0_All_t)
{
  TestSignal0 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  // 2012/02/13 CG This explicit reference to base clase below is
  // necessary because std::ref(os) fails due to a private typedef
  // output_test_stream::result_type (in Boost <=1.48.0) screwing up
  // std::ref's attempt to determine whether output_test_stream is a
  // callable entity.
  std::ostringstream & osr(os);
  BOOST_CHECK_NO_THROW(s.watchAll([&osr, &test_text](){ testCallback<0>(osr, test_text); }));
  BOOST_CHECK_NO_THROW(s.invoke(sID));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(watchFail)
{
  TestSignal0 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  // 2012/02/13 CG This explicit reference to base clase below is
  // necessary because std::ref(os) fails due to a private typedef
  // output_test_stream::result_type (in Boost <=1.48.0) screwing up
  // std::ref's attempt to determine whether output_test_stream is a
  // callable entity.
  std::ostringstream & osr(os);
  BOOST_CHECK_THROW((s.watch(art::ScheduleID(4),
                             [&osr, &test_text](){ testCallback<0>(osr, test_text); })),
        std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
