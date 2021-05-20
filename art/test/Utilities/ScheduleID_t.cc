#define BOOST_TEST_MODULE (ScheduleID_t)
#include "boost/test/unit_test.hpp"

#include "art/Utilities/ScheduleID.h"

BOOST_AUTO_TEST_SUITE(ScheduleID_t)

BOOST_AUTO_TEST_CASE(construct)
{
  BOOST_CHECK_NO_THROW(art::ScheduleID(0));
  BOOST_CHECK_NO_THROW(art::ScheduleID(1));
  BOOST_TEST(!art::ScheduleID().isValid());
  BOOST_TEST(art::ScheduleID::first().isValid());
  BOOST_TEST(art::ScheduleID::last().isValid());
}

BOOST_AUTO_TEST_CASE(compare)
{
  BOOST_TEST(art::ScheduleID(57) == art::ScheduleID(57));
  BOOST_TEST(art::ScheduleID(57) != art::ScheduleID(58));
  BOOST_TEST(art::ScheduleID::first() < art::ScheduleID::last());
  BOOST_TEST(!(art::ScheduleID::last() < art::ScheduleID::first()));
  BOOST_TEST(art::ScheduleID::first() <= art::ScheduleID::last());
  BOOST_TEST(art::ScheduleID::first() <= art::ScheduleID::first());
  BOOST_TEST(!(art::ScheduleID::last() <= art::ScheduleID::first()));
  BOOST_TEST(art::ScheduleID::last() > art::ScheduleID::first());
  BOOST_TEST(!(art::ScheduleID::first() > art::ScheduleID::last()));
  BOOST_TEST(art::ScheduleID::last() >= art::ScheduleID::first());
  BOOST_TEST(art::ScheduleID::last() >= art::ScheduleID::last());
  BOOST_TEST(!(art::ScheduleID::first() >= art::ScheduleID::last()));
}

BOOST_AUTO_TEST_CASE(id)
{
  art::ScheduleID sID{61};
  BOOST_TEST(sID.id() == 61);
}

BOOST_AUTO_TEST_SUITE_END()
