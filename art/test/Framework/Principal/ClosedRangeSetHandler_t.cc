#define BOOST_TEST_MODULE (ClosedRangeSetHandler_t)
#include "cetlib/quiet_unit_test.hpp"

//======================================================================================
// Test of ClosedRangeSetHandler
//
// Consider the following set of SubRuns and events for Run 1.  The
// last row of numbers corresponds to the RangeSet for the SubRun
// labeled above that line.  Note that there is a discontinuity in the
// SubRun 1 event ranges.
//
//   -------------- --------------------------------
//   ------------------------------
//  | Run 1        : (Run 1)                        : (Run 1) | | SubRun 0     |
//  SubRun 1                       | SubRun 2                     | | 5 6 7 8 9
//  10 | 1 2 3 4 5 6 : 9 10 11 12 13 14 | 3 4 5 6 7 8 9 10 11 12 13 14 |
//   -------------- --------------------------------
//   ------------------------------
//
// For Run 1, the RangeSet corresponding to the above diagram would be
// printed out as:
//
//   Run 1:
//    SubRun: 0 Event range: [5,11)
//    SubRun: 1 Event range: [1,7)
//    SubRun: 1 Event range: [9,15)
//    SubRun: 2 Event range: [3,15)
//
// This test exercises the system by simulating a given number of
// processed events, and then retrieving the 'seen' event ranges based
// on when a file switch is simulated.
//
// We test three situations:
//
// (1) No events processed -- i.e. the RangeSets as shown above should
//     be forwarded to an output file with no splitting.
//
// (2) Events with Run = 1, SubRun = 1 and Event = 3, 4, 5, and 10 are
//     processed, with a file switch after event 5.
//
// (3) Events with Run = 1, SubRun = 1 and Event = 3, 4, and 5 are
//     processed, with a file switch after event 5.
//======================================================================================

#include <iostream>
#include <vector>

#include "art/Framework/Principal/ClosedRangeSetHandler.h"
#include "art/test/Framework/Principal/SimpleEvent.h"
#include "canvas/Persistency/Common/detail/aggregate.h"
#include "canvas/Persistency/Provenance/RangeSet.h"

using namespace art;
using arttest::SimpleEvent;

namespace {

  auto
  eventRanges(unsigned const subrun)
  {
    std::vector<EventRange> ranges;
    if (subrun == 0u) {
      ranges.emplace_back(0, 5, 11);
    } else if (subrun == 1u) {
      ranges.emplace_back(1, 1, 7);
      ranges.emplace_back(1, 9, 15);
    } else if (subrun == 2u) {
      ranges.emplace_back(2, 3, 15);
    }
    return ranges;
  }

  template <typename T>
  auto
  concatenate(std::vector<T>& l, std::vector<T> const& r)
  {
    art::detail::CanBeAggregated<std::vector<T>>::aggregate(l, r);
  }

  struct RSHandler {
    RSHandler()
    {
      std::vector<EventRange> runRanges;
      for (std::size_t i{0}; i != 3; ++i) {
        auto const& ranges = eventRanges(i);
        auto const& rs = RangeSet{1, ranges};
        srHandlers.push_back(std::make_unique<ClosedRangeSetHandler>(rs));
        concatenate(runRanges, ranges);
      }
      rHandler =
        std::make_unique<ClosedRangeSetHandler>(RangeSet{1, runRanges});
    }
    std::vector<std::unique_ptr<RangeSetHandler>>
      srHandlers; // 3 separate RangeSetHandlers
    std::unique_ptr<RangeSetHandler>
      rHandler; // 1 RangeSetHandler with all ranges
  };
}

BOOST_FIXTURE_TEST_SUITE(ClosedRangeSetHandler_t, RSHandler)

BOOST_AUTO_TEST_CASE(EmptyRunAndSubRuns)
{
  // In this case, no events were seen in any of the runs/subruns.  So
  // flush the ranges.
  for (std::size_t i{0}; i != 3; ++i) {
    srHandlers[i]->flushRanges();
    auto const& rs = RangeSet{1, eventRanges(i)};
    BOOST_CHECK_EQUAL(srHandlers[i]->seenRanges(), rs);
  }

  rHandler->flushRanges();
  std::vector<EventRange> runRef;
  for (std::size_t i{0}; i != 3; ++i) {
    concatenate(runRef, eventRanges(i));
  }
  RangeSet const runRSRef{1, runRef};
  BOOST_CHECK_EQUAL(rHandler->seenRanges(), runRSRef);
}

BOOST_AUTO_TEST_CASE(SplitOnNonLastSubRunEvent)
{
  // For this test, we're in the middle of processing SubRun 1 at
  // event 5, when we decide to switch to a new input file.  In this
  // case, 5 IS NOT the last processed event in the SubRun.

  std::vector<SimpleEvent> events;
  events.emplace_back(EventID{1, 1, 3}, false);
  events.emplace_back(EventID{1, 1, 4}, false);
  events.emplace_back(EventID{1, 1, 5}, false);

  auto const& srHandler = srHandlers[1];
  // Process events
  for (auto const& e : events) {
    srHandler->update(e.id, e.lastInSubRun);
    rHandler->update(e.id, e.lastInSubRun);
  }

  // Simulate file switch
  // 1. Maybe split range
  srHandler->maybeSplitRange();
  rHandler->maybeSplitRange();

  {
    std::vector<EventRange> subRunRef;
    subRunRef.emplace_back(1, 1, 6);
    RangeSet const subRunRSRef{1, subRunRef};
    BOOST_CHECK_EQUAL(srHandler->seenRanges(), subRunRSRef);
  }

  {
    std::vector<EventRange> runRef;
    runRef.emplace_back(0, 5, 11);
    runRef.emplace_back(1, 1, 6);
    RangeSet const runRSRef{1, runRef};
    BOOST_CHECK_EQUAL(rHandler->seenRanges(), runRSRef);
  }

  // 2. Rebase ranges
  srHandler->rebase();
  rHandler->rebase();

  // Process last event
  SimpleEvent const lastEvent{EventID{1, 1, 10}, true};
  srHandler->update(lastEvent.id, lastEvent.lastInSubRun);
  rHandler->update(lastEvent.id, lastEvent.lastInSubRun);

  // Simulate end of subrun and run
  srHandler->flushRanges();
  rHandler->flushRanges();
  {
    std::vector<EventRange> subRunRef;
    subRunRef.emplace_back(1, 6, 7);
    subRunRef.emplace_back(1, 9, 15);
    RangeSet const subRunRSRef{1, subRunRef};
    BOOST_CHECK_EQUAL(srHandler->seenRanges(), subRunRSRef);
  }
  {
    std::vector<EventRange> runRef;
    runRef.emplace_back(1, 6, 7);
    runRef.emplace_back(1, 9, 15);
    runRef.emplace_back(2, 3, 15);
    RangeSet const runRSRef{1, runRef};
    BOOST_CHECK_EQUAL(rHandler->seenRanges(), runRSRef);
  }
}

BOOST_AUTO_TEST_CASE(SplitOnLastSubRunEvent)
{
  // For this test, we're in the middle of processing SubRun 1 at
  // event 5, when we decide to switch to a new input file.  In this
  // case, 5 IS the last processed event in the SubRun.

  std::vector<SimpleEvent> events;
  events.emplace_back(EventID{1, 1, 3}, false);
  events.emplace_back(EventID{1, 1, 4}, false);
  events.emplace_back(EventID{1, 1, 5}, true);

  auto const& srHandler = srHandlers[1];
  // Process events
  for (auto const& e : events) {
    srHandler->update(e.id, e.lastInSubRun);
    rHandler->update(e.id, e.lastInSubRun);
  }

  // Simulate file switch
  srHandler->maybeSplitRange();
  rHandler->maybeSplitRange();
  {
    RangeSet const subRunRSRef{1, eventRanges(1)};
    BOOST_CHECK_EQUAL(srHandler->seenRanges(), subRunRSRef);
  }
  {
    std::vector<EventRange> runRef;
    concatenate(runRef, eventRanges(0));
    concatenate(runRef, eventRanges(1));
    RangeSet const runRSRef{1, runRef};
    BOOST_CHECK_EQUAL(rHandler->seenRanges(), runRSRef);
  }
  srHandler->rebase();
  rHandler->rebase();

  // Both RangeSets should now be empty (have not yet processed any
  // events from SubRun 2).
  RangeSet const emptyRS{1, std::vector<EventRange>{}};
  BOOST_CHECK_EQUAL(srHandler->seenRanges(), emptyRS);
  BOOST_CHECK_EQUAL(rHandler->seenRanges(), emptyRS);
}

BOOST_AUTO_TEST_SUITE_END()
