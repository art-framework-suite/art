#define BOOST_TEST_MODULE(EventID_t)
#include "boost/test/auto_unit_test.hpp"

#include "art/Persistency/Provenance/EventID.h"

using namespace art;

BOOST_AUTO_TEST_SUITE(EventID_t)

BOOST_AUTO_TEST_CASE(DefaultConstruction)
{
  EventID def;
  BOOST_REQUIRE(!def.isValid());
  BOOST_REQUIRE(!def.subRunID().isValid());
  BOOST_REQUIRE(!def.runID().isValid());
}

BOOST_AUTO_TEST_CASE(FullSpecConstruction)
{
  EventNumber_t const et = 1;
  RunNumber_t const rt = 2;
  SubRunNumber_t const srt = 5;
  EventID good(rt, srt, et);
  BOOST_REQUIRE_EQUAL(good.run(), rt);
  BOOST_REQUIRE_EQUAL(good.subRun(), srt);
  BOOST_REQUIRE_EQUAL(good.event(), et);
}

BOOST_AUTO_TEST_CASE(Flush)
{
  EventID f(EventID::flushEvent());
  BOOST_REQUIRE(f.isValid());
  BOOST_REQUIRE(f.isFlush());
  BOOST_REQUIRE(f.subRunID().isFlush());
  BOOST_REQUIRE(f.runID().isFlush());

  EventID fs(EventID::flushEvent(RunID::firstRun()));
  BOOST_REQUIRE(fs.isValid());
  BOOST_REQUIRE(fs.isFlush());
  BOOST_REQUIRE(fs.subRunID().isFlush());
  BOOST_REQUIRE(!fs.runID().isFlush());
  BOOST_REQUIRE_EQUAL(fs.run(), RunID::firstRun().run());

  BOOST_REQUIRE_THROW(EventID(f.run(), f.subRun(), f.event()), art::Exception);
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  EventID const small(1, 1, 1);
  EventID const med(2, 2, 2);
  EventID const med2(2, 2, 2);
  EventID const large(3, 3, 2);
  EventID const largest(3, 3, 3);
  BOOST_REQUIRE(small < med);
  BOOST_REQUIRE(small <= med);
  BOOST_REQUIRE(!(small == med));
  BOOST_REQUIRE(small != med);
  BOOST_REQUIRE(!(small > med));
  BOOST_REQUIRE(!(small >= med));
  BOOST_REQUIRE(med2 == med);
  BOOST_REQUIRE(med2 <= med);
  BOOST_REQUIRE(med2 >= med);
  BOOST_REQUIRE(!(med2 != med));
  BOOST_REQUIRE(!(med2 < med));
  BOOST_REQUIRE(!(med2 > med));
  BOOST_REQUIRE(med < large);
  BOOST_REQUIRE(med <= large);
  BOOST_REQUIRE(!(med == large));
  BOOST_REQUIRE(med != large);
  BOOST_REQUIRE(!(med > large));
  BOOST_REQUIRE(!(med >= large));
  BOOST_REQUIRE(large < largest);
  BOOST_REQUIRE(large <= largest);
  BOOST_REQUIRE(!(large == largest));
  BOOST_REQUIRE(large != largest);
  BOOST_REQUIRE(!(large > largest));
  BOOST_REQUIRE(!(large >= largest));
}

BOOST_AUTO_TEST_CASE(Iteration)
{
  EventID first = EventID::firstEvent();
  EventID second = first.next();
  BOOST_REQUIRE(first < second);
  BOOST_REQUIRE(first == (second.previous()));
  EventID run2(2, 1, 1);
  BOOST_REQUIRE(run2 < run2.nextRun());
  BOOST_REQUIRE(run2 < run2.nextSubRun());

  EventID largeEvent(SubRunID::firstSubRun(), EventID::maxEvent().event());
  BOOST_REQUIRE(largeEvent.isValid());
  EventID next(largeEvent.next());
  BOOST_REQUIRE_EQUAL(SubRunID::firstSubRun().next(), next.subRunID());
  BOOST_REQUIRE_EQUAL(next.event(), EventID::firstEvent().event());
  BOOST_REQUIRE(next.isValid());
  BOOST_REQUIRE(!next.isFlush());
}

BOOST_AUTO_TEST_SUITE_END()
