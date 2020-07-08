#define BOOST_TEST_MODULE (GlobalSignal_t)
#include "boost/test/tools/output_test_stream.hpp"
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Services/Registry/GlobalSignal.h"

#include <ostream>
#include <string>

namespace {

  using TestSignal0 =
    art::GlobalSignal<art::detail::SignalResponseType::FIFO, void()>;
  using TestSignal1 = art::GlobalSignal<art::detail::SignalResponseType::FIFO,
                                        void(std::ostream&)>;
  using TestSignal2 =
    art::GlobalSignal<art::detail::SignalResponseType::FIFO,
                      void(std::ostream&, std::string const&)>;
  using TestSignal2a =
    art::GlobalSignal<art::detail::SignalResponseType::LIFO,
                      void(std::ostream&, std::string const&)>;

  template <uint16_t n>
  void
  testCallback(std::ostream& os, std::string const& text)
  {
    if (n > 0) {
      os << n << ": ";
    }
    os << text;
  }
  struct CallBackClass {
    void
    func(std::ostream& os, std::string const& text)
    {
      os << text;
    }
    void
    cfunc(std::ostream& os, std::string const& text) const
    {
      os << text;
    }
  };
} // namespace

BOOST_AUTO_TEST_SUITE(GlobalSignal_t)

BOOST_AUTO_TEST_CASE(TestSignal2_t)
{
  TestSignal2 s;
  std::string const test_text{"Test text.\n"};
  boost::test_tools::output_test_stream os;
  s.watch(testCallback<1>);
  s.watch(testCallback<2>);
  s.watch(testCallback<3>);
  std::string const cmp_text{std::string("1: ") + test_text +
                             "2: " + test_text + "3: " + test_text};
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_TEST(os.is_equal(cmp_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2a_t)
{
  TestSignal2a s;
  std::string const test_text{"Test text.\n"};
  boost::test_tools::output_test_stream os;
  s.watch(testCallback<1>);
  s.watch(testCallback<2>);
  s.watch(testCallback<3>);
  std::string const cmp_text{std::string("3: ") + test_text +
                             "2: " + test_text + "1: " + test_text};
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_TEST(os.is_equal(cmp_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_func_t)
{
  TestSignal2 s;
  std::string const test_text{"Test text"};
  boost::test_tools::output_test_stream os;
  CallBackClass cbc;
  s.watch(&CallBackClass::func, cbc);
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_TEST(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal2_cfunc_t)
{
  TestSignal2 s;
  std::string const test_text{"Test text"};
  boost::test_tools::output_test_stream os;
  CallBackClass const cbc;
  s.watch(&CallBackClass::cfunc, cbc);
  BOOST_CHECK_NO_THROW(s.invoke(os, test_text));
  BOOST_TEST(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal1_t)
{
  TestSignal1 s;
  std::string const test_text{"Test text"};
  boost::test_tools::output_test_stream os;
  s.watch([&test_text](auto& x) { testCallback<0>(x, test_text); });
  BOOST_CHECK_NO_THROW(s.invoke(os));
  BOOST_TEST(os.is_equal(test_text));
}

BOOST_AUTO_TEST_CASE(TestSignal0_t)
{
  TestSignal0 s;
  std::string const test_text{"Test text"};
  boost::test_tools::output_test_stream os;
  // 2012/02/13 CG This explicit reference to base clase below is
  // necessary because std::ref(os) fails due to a private typedef
  // output_test_stream::result_type (in Boost <=1.53.0 at least)
  // screwing up std::ref's attempt to determine whether
  // output_test_stream is a callable entity.
  std::ostringstream& osr [[maybe_unused]]{os};
  s.watch([&osr, &test_text] { testCallback<0>(osr, test_text); });
  BOOST_CHECK_NO_THROW(s.invoke());
  BOOST_TEST(os.is_equal(test_text));
}

BOOST_AUTO_TEST_SUITE_END()
