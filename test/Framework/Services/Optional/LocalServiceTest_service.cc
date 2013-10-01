#include "test/Framework/Services/Optional/LocalServiceTest.h"

arttest::LocalServiceTest::LocalServiceTest(art::ScheduleID sID)
  :
  sID_(sID)
{
}

arttest::LocalServiceTest::LocalServiceTest(fhicl::ParameterSet const &,
                        art::ActivityRegistry &,
                        art::ScheduleID sID)
  :
  LocalServiceTest(sID)
{
}

DEFINE_ART_SERVICE(arttest::LocalServiceTest)
