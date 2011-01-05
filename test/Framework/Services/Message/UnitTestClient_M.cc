#include "test/Framework/Services/Message/UnitTestClient_M.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>
#include <iomanip>

// Test of LogSystem, LogAbsolute, LogProblem, LogPrint, LogVerbatim

namespace arttest
{


void
  UnitTestClient_M::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  art::LogSystem("system")    <<
  	"Text sent to LogSystem";
  art::LogAbsolute("absolute")  <<
  	"Text sent to LogAbsolute - should be unformatted";
  art::LogProblem("problem")   <<
  	"Text sent to LogProblem - should be unformatted";
  art::LogPrint("print")       <<
  	"Text sent to LogPrint- should be unformatted";
  art::LogVerbatim("verbatim") <<
  	"Text sent to LogVerbatim - should be unformatted";
}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_M;
DEFINE_FWK_MODULE(UnitTestClient_M);
