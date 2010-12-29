#include "test/Framework/Services/Message/UnitTestClient_K.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_K::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{

  for (int i=0; i<10; ++i) {
    art::LogPrint  ("cat_P") << "LogPrint: " << i;
    art::LogSystem ("cat_S") << "LogSystem: " << i;
  }

}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_K;
DEFINE_FWK_MODULE(UnitTestClient_K);
