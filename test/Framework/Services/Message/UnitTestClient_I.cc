#include "test/Framework/Services/Message/UnitTestClient_I.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_I::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  std::string empty_;
  std::string file_ = "nameOfFile";
       LogDebug  ("cat_A")   << "LogDebug was used to send this message";
       LogDebug  ("cat_B")   << "LogDebug was used to send this other message";
  art::LogError  ("cat_A")   << "LogError was used to send this message"
  			     << "-which is long enough to span lines but-"
			     << "will not be broken up by the logger any more";
  art::LogError  ("cat_B")   << "LogError was used to send this other message";
  art::LogWarning("cat_A")   << "LogWarning was used to send this message";
  art::LogWarning("cat_B")   << "LogWarning was used to send this other message";
  art::LogInfo   ("cat_A")   << "LogInfo was used to send this message";
  art::LogInfo   ("cat_B")   << "LogInfo was used to send this other message";

  art::LogInfo   ("FwkJob")  << "<Message>LogInfo was used to send a job report</Message>";

 }  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_I;
DEFINE_FWK_MODULE(UnitTestClient_I);
