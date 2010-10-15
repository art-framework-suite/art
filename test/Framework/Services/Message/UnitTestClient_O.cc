#include "test/Framework/Services/Message/UnitTestClient_O.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_O::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  art::LogInfo   ("importantInfo")
  		<< "This LogInfo message should appear in both destinations";
  art::LogInfo   ("routineInfo")
		<< "This LogInfo message should appear in the info destination";


 }  // MessageLoggerClient::analyze()


}  // namespace arttest


using arttest::UnitTestClient_O;
DEFINE_FWK_MODULE(UnitTestClient_O);
