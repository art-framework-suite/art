#include "test/Framework/Services/Message/UnitTestClient_Q.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace arttest
{

void
  UTC_Q1::analyze( art::Event      const & e
                            , art::EventSetup const & /*unused*/
                              )
{
  art::LogInfo   ("cat_A")   << "Q1 with identifier " << identifier;
  art::LogInfo   ("timer")   << "Q1 timer with identifier " << identifier;
  art::LogInfo   ("trace")   << "Q1 trace with identifier " << identifier;
}

void
  UTC_Q2::analyze( art::Event      const & e
                            , art::EventSetup const & /*unused*/
                              )
{
  art::LogInfo   ("cat_A")   << "Q2 with identifier " << identifier;
  art::LogInfo   ("timer")   << "Q2 timer with identifier " << identifier;
  art::LogInfo   ("trace")   << "Q2 trace with identifier " << identifier;
}


}  // arttest

using arttest::UTC_Q1;
using arttest::UTC_Q2;
DEFINE_FWK_MODULE(UTC_Q1);
DEFINE_FWK_MODULE(UTC_Q2);
