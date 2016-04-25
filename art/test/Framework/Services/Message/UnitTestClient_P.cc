// Tests effect of LogFlush by cfg-configurable choices of how many
// messages to use to clog the queue and whether or not FlushMessageLog
// is invoked.  Under normal testing, it will invoke FlushMessageLog in
// a situation where its absence would result in different output.

#include "art/test/Framework/Services/Message/UnitTestClient_P.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_P::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  mf::LogWarning ("configuration") << "useLogFlush = " << useLogFlush
                               << " queueFillers = " << queueFillers;
  std::string longMessage;
  for (int k=0; k<100; k++) {
    longMessage += "Line in long message\n";
  }
  for (int i=0; i< queueFillers; ++i) {
    mf::LogInfo("cat") <<  "message " << i << "\n" << longMessage;
  }

  mf::LogError ("keyMessage") << "This message is issued just before abort";
  if  (useLogFlush)  art::FlushMessageLog();
  abort();

 }  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_P;
DEFINE_ART_MODULE(UnitTestClient_P)
