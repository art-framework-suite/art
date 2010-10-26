#include "test/Framework/Services/Message/MessageLoggerClient.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

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
  art::LogInfo("aTestMessage") << "LogInfo was used to send this message";
  art::LogWarning("aTestMessage") << "LogWarning was used to send this message";
  art::LogError("aTestMessage") << "LogError was used to send this message";
  art::LogInfo("cat1|cat2||cat3") << "Three-category message";

  art::LogWarning("aboutToSend") << "about to send 100 warnings";
  for( unsigned i = 0;  i != 100;  ++i )  {
    art::LogWarning("unimportant") << "warning number " << i;
  }


}  // MessageLoggerClient::analyze()


}  // namespace arttest


using arttest::MessageLoggerClient;
DEFINE_FWK_MODULE(MessageLoggerClient);
