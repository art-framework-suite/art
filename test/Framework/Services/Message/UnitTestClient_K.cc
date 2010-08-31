#include "test/Framework/Services/Message/UnitTestClient_K.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace edmtest
{


void
  UnitTestClient_K::analyze( edm::Event      const & e
                           , edm::EventSetup const & /*unused*/
                              )
{

  for (int i=0; i<10; ++i) {
    edm::LogPrint  ("cat_P") << "LogPrint: " << i;
    edm::LogSystem ("cat_S") << "LogSystem: " << i;
  }

}  // MessageLoggerClient::analyze()


}  // namespace edmtest


using edmtest::UnitTestClient_K;
DEFINE_FWK_MODULE(UnitTestClient_K);
