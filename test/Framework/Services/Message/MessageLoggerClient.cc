#include "test/Framework/Services/Message/MessageLoggerClient.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>


namespace arttest
{


void
  MessageLoggerClient::analyze( art::Event      const & e
                              , art::EventSetup const & /*unused*/
                              )
{
  //std::cout << "Module reached\n";
  LogDebug("aTestMessage") << "LogDebug was used to send this message";
  mf::LogInfo("aTestMessage") << "LogInfo was used to send this message";
  mf::LogWarning("aTestMessage") << "LogWarning was used to send this message";
  mf::LogError("aTestMessage") << "LogError was used to send this message";
  mf::LogInfo("cat1|cat2||cat3") << "Three-category message";

  mf::LogWarning("aboutToSend") << "about to send 100 warnings";
  for( unsigned i = 0;  i != 100;  ++i )  {
    mf::LogWarning("unimportant") << "warning number " << i;
  }


}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::MessageLoggerClient;
DEFINE_ART_MODULE(MessageLoggerClient);
