#define BOOST_TEST_MODULE(EventID_t)
#include "boost/test/auto_unit_test.hpp"

#include "art/Persistency/Provenance/EventID.h"

using namespace art;

BOOST_AUTO_TEST_SUITE(EventID_t)

BOOST_AUTO_TEST_CASE(construction)
{
  const EventNumber_t et = 1;
  const RunNumber_t rt = 2;
  const SubRunNumber_t srt = 5;
  EventID temp(rt, srt, et);
  BOOST_REQUIRE(temp.run() == rt);
  BOOST_REQUIRE(temp.subRun() == srt);
  BOOST_REQUIRE(temp.event() == et);
}

BOOST_AUTO_TEST_CASE(comparison)
{
  const EventID small(1, 1, 1);
  const EventID med(2, 2, 2);
  const EventID med2(2, 2, 2);
  const EventID large(3, 3, 2);
  const EventID largest(3, 3, 3);
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

BOOST_AUTO_TEST_CASE(iteration)
{
  EventID first = EventID::firstEvent();
  EventID second = first.next();
  BOOST_REQUIRE(first < second);
  BOOST_REQUIRE(first == (second.previous()));
  EventID run2(2, 1, 1);
  BOOST_REQUIRE(run2 < run2.nextRun());
  BOOST_REQUIRE(run2 < run2.nextSubRun());
}

BOOST_AUTO_TEST_CASE(flush)
{
}

BOOST_AUTO_TEST_SUITE_END()
