#define BOOST_TEST_MODULE (GlobalSignal_t)
#include "boost/test/auto_unit_test.hpp"
#include "boost/test/output_test_stream.hpp"

#include "art/Framework/Services/Registry/GlobalSignal.h"

#include <ostream>
#include <string>

namespace {
  typedef art::GlobalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &, std::string const &> TestSignal2;
  typedef art::GlobalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &> TestSignal1;
  typedef art::GlobalSignal<art::detail::SignalResponseType::FIFO, void> TestSignal0;
  void
  testCallback(std::ostream & os, std::string const & text)
  {
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
}

BOOST_AUTO_TEST_SUITE(GlobalSignal_t)

BOOST_AUTO_TEST_CASE(TestSignal2_t)
{
  TestSignal2 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  s.watch(testCallback);
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_func_t)
{
  TestSignal2 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  CallBackClass cbc;
  s.watch(&CallBackClass::func, cbc);
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_cfunc_t)
{
  TestSignal2 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  CallBackClass const cbc;
  s.watch(&CallBackClass::cfunc, cbc);
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_clear_t)
{
  TestSignal2 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  s.watch(testCallback);
  s.clear();
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_CHECK(os.is_empty());
}

BOOST_AUTO_TEST_CASE(TestSignal1_t)
{
  TestSignal1 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  s.watch(std::bind(testCallback, std::placeholders::_1, std::cref(test_text)));
  BOOST_CHECK_NO_THROW(s.invoke(os));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal0_t)
{
  TestSignal0 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  // 2012/02/13 CG This explicit reference to base clase below is
  // necessary because std::ref(os) fails due to a private typedef
  // output_test_stream::result_type (in Boost <=1.53.0 at least)
  // screwing up std::ref's attempt to determine whether
  // output_test_stream is a callable entity.
  std::ostringstream & osr __attribute__((unused))(os);
  s.watch(std::bind(testCallback, std::ref(osr), std::cref(test_text)));
  BOOST_CHECK_NO_THROW(s.invoke());
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_SUITE_END()
