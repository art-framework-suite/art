#include "art/test/Framework/Services/Message/UnitTestClient_K.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

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
    mf::LogPrint  ("cat_P") << "LogPrint: " << i;
    mf::LogSystem ("cat_S") << "LogSystem: " << i;
  }

}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_K;
DEFINE_ART_MODULE(UnitTestClient_K)
