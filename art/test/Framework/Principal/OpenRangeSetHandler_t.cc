// Test of OpenRangeSetHandler

#define BOOST_TEST_MODULE (OpenRangeSetHandler_t)
#include "boost/test/unit_test.hpp"

#include <iostream>
#include <vector>

#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/test/Framework/Principal/SimpleEvent.h"
#include "canvas/Persistency/Common/detail/aggregate.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

using namespace art;
using namespace std;
using arttest::SimpleEvent;

namespace {

  // Event ranges:
  //   SubRun: 0 Event range: [1,4)
  //   SubRun: 1 Event range: [1,4)
  //   SubRun: 2 Event range: [1,4)
  auto
  eventRanges(unsigned const sr)
  {
    vector<EventRange> const ranges{EventRange{sr, 1, 4}};
    return ranges;
  }

  template <typename T>
  auto
  concatenate(vector<T>& l, vector<T> const& r)
  {
    art::detail::CanBeAggregated<vector<T>>::aggregate(l, r);
  }

  struct RSHandler {
    RSHandler()
    {
      // Run 1 SubRuns 0-2
      for (unsigned sr{0}; sr != 3; ++sr) {
        events.emplace_back(EventID{1, sr, 1}, false);
        events.emplace_back(EventID{1, sr, 2}, false);
        events.emplace_back(EventID{1, sr, 3}, true);
      }
    }
    vector<SimpleEvent> events;
    unique_ptr<RangeSetHandler> srHandler = make_unique<OpenRangeSetHandler>(1);
    unique_ptr<RangeSetHandler> rHandler = make_unique<OpenRangeSetHandler>(1);
  };
} // namespace

BOOST_FIXTURE_TEST_SUITE(OpenRangeSetHandler_t, RSHandler)

BOOST_AUTO_TEST_CASE(Simple)
{
  // RangeSet reference

  // Process events
  for (auto const& e : events) {
    rHandler->update(e.id, e.lastInSubRun);
    srHandler->update(e.id, e.lastInSubRun);
    if (e.lastInSubRun) {
      srHandler->flushRanges();
      {
        RangeSet subRunRS{1};
        subRunRS.emplace_range(e.id.subRun(), 1, 4);
        BOOST_TEST(srHandler->seenRanges() == subRunRS);
      }
      srHandler = make_unique<OpenRangeSetHandler>(1); // New subrun
    }
  }

  vector<EventRange> ranges;
  for (unsigned sr{0}; sr != 3; ++sr) {
    concatenate(ranges, eventRanges(sr));
  }
  RangeSet const runRS{1, ranges};
  BOOST_TEST(rHandler->seenRanges() == runRS);
}

BOOST_AUTO_TEST_CASE(FileSwitchAfterEvent)
{
  // Process 5 events
  for (std::size_t i{0}; i != 5; ++i) {
    auto const& e = events[i];
    srHandler->update(e.id, e.lastInSubRun);
    rHandler->update(e.id, e.lastInSubRun);
    if (e.lastInSubRun) {
      srHandler->flushRanges();
      {
        RangeSet subRunRS{1};
        subRunRS.emplace_range(e.id.subRun(), 1, 4);
        BOOST_TEST(srHandler->seenRanges() == subRunRS);
      }
      srHandler = make_unique<OpenRangeSetHandler>(1); // New subrun
    }
  }

  // Simulate file-switch
  srHandler->maybeSplitRange();
  rHandler->maybeSplitRange();
  {
    vector<EventRange> runRanges{EventRange{0, 1, 4}};
    vector<EventRange> subRunRanges{EventRange{1, 1, 3}};
    concatenate(runRanges, subRunRanges);
    RangeSet const subRunRS{1, subRunRanges};
    RangeSet const runRS{1, runRanges};
    BOOST_TEST(srHandler->seenRanges() == subRunRS);
    BOOST_TEST(rHandler->seenRanges() == runRS);
  }
  srHandler->rebase();
  rHandler->rebase();

  // Process remaining events
  // ... first do rest of SubRun 1
  auto const& e = events[5];
  srHandler->update(e.id, e.lastInSubRun);
  rHandler->update(e.id, e.lastInSubRun);
  srHandler->flushRanges();
  {
    RangeSet subRunRS{1};
    subRunRS.emplace_range(1, 3, 4);
    BOOST_TEST(srHandler->seenRanges() == subRunRS);
  }
  srHandler = make_unique<OpenRangeSetHandler>(1); // New subrun

  for (std::size_t i{6}; i != events.size(); ++i) {
    auto const& e = events[i];
    srHandler->update(e.id, e.lastInSubRun);
    rHandler->update(e.id, e.lastInSubRun);
  }

  // End job
  srHandler->flushRanges();
  rHandler->flushRanges();
  {
    vector<EventRange> runRanges{EventRange{1, 3, 4}};
    vector<EventRange> subRunRanges{EventRange{2, 1, 4}};
    concatenate(runRanges, subRunRanges);
    RangeSet const subRunRS{1, subRunRanges};
    RangeSet const runRS{1, runRanges};
    BOOST_TEST(srHandler->seenRanges() == subRunRS);
    BOOST_TEST(rHandler->seenRanges() == runRS);
  }
}

BOOST_AUTO_TEST_CASE(FileSwitchAfterSubRun)
{
  // Process 6 events
  for (std::size_t i{0}; i != 6; ++i) {
    auto const& e = events[i];
    srHandler->update(e.id, e.lastInSubRun);
    rHandler->update(e.id, e.lastInSubRun);
    if (e.lastInSubRun) {
      srHandler->flushRanges();
      {
        RangeSet subRunRS{1};
        subRunRS.emplace_range(e.id.subRun(), 1, 4);
        BOOST_TEST(srHandler->seenRanges() == subRunRS);
      }
      srHandler = make_unique<OpenRangeSetHandler>(1); // New subrun
    }
  }

  // Simulate file-switch
  srHandler->maybeSplitRange();
  rHandler->maybeSplitRange();
  {
    vector<EventRange> runRanges;
    for (std::size_t i{0}; i != 2; ++i) {
      concatenate(runRanges, eventRanges(i));
    }
    RangeSet const subRunRS{1, std::vector<EventRange>{}};
    RangeSet const runRS{1, runRanges};
    BOOST_TEST(srHandler->seenRanges() == subRunRS);
    BOOST_TEST(rHandler->seenRanges() == runRS);
  }
  srHandler->rebase();
  rHandler->rebase();

  for (std::size_t i{6}; i != events.size(); ++i) {
    auto const& e = events[i];
    srHandler->update(e.id, e.lastInSubRun);
    rHandler->update(e.id, e.lastInSubRun);
  }

  // End job
  srHandler->flushRanges();
  rHandler->flushRanges();
  {
    vector<EventRange> const ranges{EventRange{2, 1, 4}};
    RangeSet const rs{1, ranges};
    BOOST_TEST(srHandler->seenRanges() == rs);
    BOOST_TEST(rHandler->seenRanges() == rs);
  }
}

BOOST_AUTO_TEST_SUITE_END()
