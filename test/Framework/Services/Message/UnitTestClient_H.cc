#include "test/Framework/Services/Message/UnitTestClient_H.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace edmtest
{


void
  UnitTestClient_H::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
       LogTrace    ("cat_A") << "LogTrace was used to send this mess" << "age";
       LogDebug    ("cat_B") << "LogDebug was used to send this other message";
  art::LogVerbatim ("cat_A") << "LogVerbatim was us" << "ed to send this message";
  art::LogInfo     ("cat_B") << "LogInfo was used to send this other message";
}  // MessageLoggerClient::analyze()


}  // namespace edmtest


using edmtest::UnitTestClient_H;
DEFINE_FWK_MODULE(UnitTestClient_H);
