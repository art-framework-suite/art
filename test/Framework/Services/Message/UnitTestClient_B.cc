#include "test/Framework/Services/Message/UnitTestClient_B.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{

int  UnitTestClient_B::nevent = 0;

void
  UnitTestClient_B::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  nevent++;
  for (int i = 0; i < nevent; ++i) {
    mf::LogError  ("cat_A")   << "LogError was used to send this message";
  }
  mf::LogError  ("cat_B")   << "LogError was used to send this other message";
  mf::LogStatistics();
}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_B;
DEFINE_ART_MODULE(UnitTestClient_B)
