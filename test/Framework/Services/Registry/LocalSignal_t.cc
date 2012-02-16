#define BOOST_TEST_MODULE (LocalSignal_t)
#include "boost/test/auto_unit_test.hpp"
#include "boost/test/output_test_stream.hpp"

#include "art/Framework/Services/Registry/LocalSignal.h"

#include <ostream>
#include <stdexcept>
#include <string>

namespace {
  // Need to test these three types until variadic templates work
  // correctly, in which case we'll only need the one test.
  typedef art::LocalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &, std::string const &> TestSignal2;
  typedef art::LocalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &> TestSignal1;
  typedef art::LocalSignal<art::detail::SignalResponseType::FIFO, void> TestSignal0;
  void
  testCallback(std::ostream & os, std::string const & text)
  {
    os << text;
  }

  size_t const nSchedules = 3;
  art::ScheduleID const sID { 1 };
}

BOOST_AUTO_TEST_SUITE(LocalSignal_t)

BOOST_AUTO_TEST_CASE(TestSignal2_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watch(sID, testCallback));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal1_t)
{
  TestSignal1 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watch(sID,
                               std::bind(testCallback,
                                         std::placeholders::_1,
                                         std::cref(test_text))));
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
  // output_test_stream::result_type (in Boost <=1.48.0) screwing up
  // std::ref's attempt to determine whether output_test_stream is a
  // callable entity.
  std::ostringstream & osr(os);
  BOOST_CHECK_NO_THROW(s.watch(sID,
                               std::bind(testCallback,
                                         std::ref(osr),
                                         std::cref(test_text))));
  BOOST_CHECK_NO_THROW(s.invoke(sID));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_All_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll(testCallback));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_clear_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll(testCallback));
  BOOST_CHECK_NO_THROW(s.clear(sID));
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_empty());
}

BOOST_AUTO_TEST_CASE(TestSignal2_clearAll_t)
{
  TestSignal2 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll(testCallback));
  BOOST_CHECK_NO_THROW(s.clearAll());
  BOOST_CHECK_NO_THROW(s.invoke(sID, os, test_text));
  BOOST_CHECK(os.is_empty());
}

BOOST_AUTO_TEST_CASE(TestSignal1_All_t)
{
  TestSignal1 s(nSchedules);
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  BOOST_CHECK_NO_THROW(s.watchAll(std::bind(testCallback,
                                            std::placeholders::_1,
                                            std::cref(test_text))));
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
  BOOST_CHECK_NO_THROW(s.watchAll(std::bind(testCallback,
                                            std::ref(osr),
                                            std::cref(test_text))));
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
  BOOST_CHECK_THROW((s.watch(art::ScheduleID(4), std::bind(testCallback, std::ref(osr), std::cref(test_text)))), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()
