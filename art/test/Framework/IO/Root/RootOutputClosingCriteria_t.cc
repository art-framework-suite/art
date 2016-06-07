#define BOOST_TEST_MODULE ( RootOutputClosingCriteria_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"

#include <thread>

using art::ClosingCriteria;

using file_size_t = unsigned;
using no_events_t = art::FileIndex::EntryNumber_t;
using seconds_t = std::chrono::seconds;

namespace {
  constexpr auto max_size   = std::numeric_limits<file_size_t>::max();
  constexpr auto max_events = std::numeric_limits<no_events_t>::max();
  constexpr auto max_age    = seconds_t::max();

  // The minimum duration a user can specify for output-file closing
  // is 1 second.  This is different than seconds_t::min().
  constexpr seconds_t min_duration {1};

  auto const& should_close = art::criteriaMet;

  auto makeClosingCriteria(file_size_t const fileSize,
                           no_events_t const events,
                           seconds_t const secs)
  {
    ClosingCriteria c {};
    c.maxFileSize = fileSize;
    c.maxEventsPerFile = events;
    c.maxFileAge = secs;
    return c;
  }

}

BOOST_AUTO_TEST_SUITE(RootOutputClosingCriteria_t)

BOOST_AUTO_TEST_CASE(DefaultCriteria)
{
  ClosingCriteria c {};
  BOOST_CHECK(!should_close(c, 0, 0, min_duration));
}

BOOST_AUTO_TEST_CASE(TwoSecondSleep)
{
  using namespace std::chrono;
  auto const c = makeClosingCriteria(max_size, max_events, min_duration);

  auto const beginTime = steady_clock::now();
  std::this_thread::sleep_for(seconds_t{2});
  auto const age = steady_clock::now()-beginTime;

  BOOST_CHECK(should_close(c, 0, 0, duration_cast<seconds>(age)));
}

BOOST_AUTO_TEST_SUITE_END()
