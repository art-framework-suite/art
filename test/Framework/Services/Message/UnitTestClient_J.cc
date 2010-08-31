#include "test/Framework/Services/Message/UnitTestClient_J.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/MessageLogger/MessageDrop.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace edmtest
{


void
  UnitTestClient_J::analyze( edm::Event      const & e
                           , edm::EventSetup const & /*unused*/
                              )
{

  edm::MessageDrop::instance()->debugEnabled  = false;

       LogTrace    ("cat_A") << "LogTrace was used to send this mess" << "age";
       LogDebug    ("cat_B") << "LogDebug was used to send this other message";
  edm::LogVerbatim ("cat_A") << "LogVerbatim was us" << "ed to send this message";
  if( edm::isInfoEnabled() )
     edm::LogInfo  ("cat_B") << "LogInfo was used to send this other message";
}  // MessageLoggerClient::analyze()


}  // namespace edmtest


using edmtest::UnitTestClient_J;
DEFINE_FWK_MODULE(UnitTestClient_J);
