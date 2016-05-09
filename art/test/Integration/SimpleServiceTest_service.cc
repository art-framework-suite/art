////////////////////////////////////////////////////////////////////////
// Class:       SimpleServiceTest
// Plugin Type: service (art v1_19_00_rc3)
// File:        SimpleServiceTest_service.cc
//
// Generated at Mon May  9 16:37:35 2016 by Christopher Green using cetskelgen
// from cetlib version v1_17_04.
////////////////////////////////////////////////////////////////////////

#include "SimpleServiceTest.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"

arttest::SimpleServiceTest::SimpleServiceTest(fhicl::ParameterSet const &)
{
}

bool
arttest::SimpleServiceTest::verifyStatus() const
{
  return true;
}

DEFINE_ART_SERVICE(arttest::SimpleServiceTest)
