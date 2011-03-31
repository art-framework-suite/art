#include "test/Framework/Services/Message/UnitTestClient_M.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
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
  mf::LogSystem("system")    <<
  	"Text sent to LogSystem";
  mf::LogAbsolute("absolute")  <<
  	"Text sent to LogAbsolute - should be unformatted";
  mf::LogProblem("problem")   <<
  	"Text sent to LogProblem - should be unformatted";
  mf::LogPrint("print")       <<
  	"Text sent to LogPrint- should be unformatted";
  mf::LogVerbatim("verbatim") <<
  	"Text sent to LogVerbatim - should be unformatted";
}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_M;
DEFINE_ART_MODULE(UnitTestClient_M);
