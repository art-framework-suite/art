#include "test/Framework/Services/Message/UnitTestClient_A.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_A::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  std::string empty_;
  std::string file_ = "nameOfFile";
       LogDebug  ("cat_A")   << "LogDebug was used to send this message";
       LogDebug  ("cat_B")   << "LogDebug was used to send this other message";
  mf::LogError  ("cat_A")   << "LogError was used to send this message"
                             << "-which is long enough to span lines but-"
                             << "will not be broken up by the logger any more";
  mf::LogError  ("cat_B")   << "LogError was used to send this other message";
  mf::LogWarning("cat_A")   << "LogWarning was used to send this message";
  mf::LogWarning("cat_B")   << "LogWarning was used to send this other message";
  mf::LogInfo   ("cat_A")   << "LogInfo was used to send this message";
  mf::LogInfo   ("cat_B")   << "LogInfo was used to send this other message";
  mf::LogInfo   ("FwkJob")  << "<Message>LogInfo was used to send a job report</Message>";

 }  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_A;
DEFINE_ART_MODULE(UnitTestClient_A)
