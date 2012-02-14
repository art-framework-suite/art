#define BOOST_TEST_MODULE (GlobalSignal_t)
#include "boost/test/auto_unit_test.hpp"
#include "boost/test/output_test_stream.hpp"

#include "art/Framework/Services/Registry/GlobalSignal.h"

#include <ostream>
#include <string>

namespace {
  // Need to test these three types until variadic templates work
  // correctly, in which case we'll only need the one test.
  typedef art::GlobalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &, std::string const &> TestSignal2;
  typedef art::GlobalSignal<art::detail::SignalResponseType::FIFO, void, std::ostream &> TestSignal1;
  typedef art::GlobalSignal<art::detail::SignalResponseType::FIFO, void> TestSignal0;
  void
  testCallback(std::ostream & os, std::string const & text)
  {
    os << text;
  }
}

// Required to test private functions:
namespace art {
  class ActivityRegistry {
  public:
    template <typename SIGNAL>
    static
    void
    invoke(SIGNAL const & s, std::ostream & os, std::string const & text) {
      s.invoke(os, text);
    }

    template <typename SIGNAL>
    static
    void
    invoke(SIGNAL const & s, std::ostream & os) {
      s.invoke(os);
    }

    template <typename SIGNAL>
    static
    void
    invoke(SIGNAL const & s) {
      s.invoke();
    }

    template <typename SIGNAL>
    static
    void clear(SIGNAL & s) {
      s.clear();
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
  BOOST_CHECK_NO_THROW(art::ActivityRegistry::invoke(s, os, test_text));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal1_t)
{
  TestSignal1 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  s.watch(std::bind(testCallback, std::placeholders::_1, std::cref(test_text)));
  BOOST_CHECK_NO_THROW(art::ActivityRegistry::invoke(s, os));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal0_t)
{
  TestSignal0 s;
  std::string const test_text { "Test text" };
  boost::test_tools::output_test_stream os;
  // 2012/02/13 CG This explicit reference to base clase below is
  // necessary because std::ref(os) fails due to a private typedef
  // output_test_stream::result_type (in Boost <=1.48.0) screwing up
  // std::ref's attempt to determine whether output_test_stream is a
  // callable entity.
  std::ostringstream & osr(os);
  s.watch(std::bind(testCallback, std::ref(osr), std::cref(test_text)));
  BOOST_CHECK_NO_THROW(art::ActivityRegistry::invoke(s));
  BOOST_CHECK(os.is_equal(test_text));
}

BOOST_AUTO_TEST_SUITE_END()
