#include "test/Framework/Services/Message/UnitTestClient_J.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "messagefacility/MessageLogger/MessageDrop.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_J::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{

  art::MessageDrop::instance()->debugEnabled  = false;

       LogTrace    ("cat_A") << "LogTrace was used to send this mess" << "age";
       LogDebug    ("cat_B") << "LogDebug was used to send this other message";
  mf::LogVerbatim ("cat_A") << "LogVerbatim was us" << "ed to send this message";
  if( art::isInfoEnabled() )
     mf::LogInfo  ("cat_B") << "LogInfo was used to send this other message";
}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_J;
DEFINE_ART_MODULE(UnitTestClient_J);
