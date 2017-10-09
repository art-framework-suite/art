#define BOOST_TEST_MODULE (RootOutputClosingCriteria_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"

#include <thread>

using art::ClosingCriteria;
using art::FileProperties;

using file_size_t = unsigned;
using no_events_t = unsigned;
using seconds_t = std::chrono::seconds;

namespace {
  constexpr auto max_size = std::numeric_limits<file_size_t>::max();
  constexpr auto max_events = std::numeric_limits<no_events_t>::max();
  constexpr auto max_age = seconds_t::max();

  // The minimum duration a user can specify for output-file closing
  // is 1 second.  This is different than seconds_t::min().
  constexpr auto
  one_second()
  {
    return seconds_t{1};
  }

} // namespace

BOOST_AUTO_TEST_SUITE(RootOutputClosingCriteria_t)

BOOST_AUTO_TEST_CASE(MaxCriteria)
{
  FileProperties const closingCriteria{
    max_events, -1u, -1u, -1u, max_size, max_age};
  ClosingCriteria c{closingCriteria, "Unset"};

  FileProperties const fp{0, 0, 0, 0, 0, one_second()};
  BOOST_CHECK(!c.should_close(fp));
}

BOOST_AUTO_TEST_CASE(TwoSecondSleep)
{
  using namespace std::chrono;
  FileProperties const closingProperties{
    max_events, -1u, -1u, -1u, max_size, one_second()};
  ClosingCriteria c{closingProperties, "Unset"};

  auto const beginTime = steady_clock::now();
  std::this_thread::sleep_for(seconds_t{2});
  auto const age = steady_clock::now() - beginTime;

  FileProperties const currentProperties{
    0, 0, 0, 0, 0, duration_cast<seconds>(age)};
  BOOST_CHECK(c.should_close(currentProperties));
}

BOOST_AUTO_TEST_SUITE_END()
