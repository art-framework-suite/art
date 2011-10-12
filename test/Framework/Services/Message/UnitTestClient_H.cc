#include "test/Framework/Services/Message/UnitTestClient_H.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_H::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
       LogTrace    ("cat_A") << "LogTrace was used to send this mess" << "age";
       LogDebug    ("cat_B") << "LogDebug was used to send this other message";
  mf::LogVerbatim ("cat_A") << "LogVerbatim was us" << "ed to send this message";
  mf::LogInfo     ("cat_B") << "LogInfo was used to send this other message";
}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_H;
DEFINE_ART_MODULE(UnitTestClient_H);
