#include "art/test/Framework/Services/Optional/PSTest.h"

arttest::PSTest::PSTest(art::ScheduleID sID) : sID_(sID) {}

arttest::PSTest::PSTest(fhicl::ParameterSet const&,
                        art::ActivityRegistry&,
                        art::ScheduleID sID)
  : PSTest(sID)
{}

DEFINE_ART_SERVICE(arttest::PSTest)
