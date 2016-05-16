#include "art/test/Framework/Services/Message/UnitTestClient_O.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_O::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  mf::LogInfo   ("importantInfo")
                << "This LogInfo message should appear in both destinations";
  mf::LogInfo   ("routineInfo")
                << "This LogInfo message should appear in the info destination";


}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_O;
DEFINE_ART_MODULE(UnitTestClient_O)
