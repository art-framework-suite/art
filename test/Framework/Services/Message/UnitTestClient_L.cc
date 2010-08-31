#include "test/Framework/Services/Message/UnitTestClient_L.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/Event.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include <iostream>
#include <string>


namespace edmtest
{


void UnitTestClient_L::analyze( edm::Event      const & e
                           , edm::EventSetup const & /*unused*/
                              )
{
  for (int i=0; i<10000000; ++i) {
  }
  edm::LogInfo     ("cat") << "Event " << e.id() << "complete";
}  // MessageLoggerClient::analyze()

void UnitTestClient_L1::analyze( edm::Event      const & e
                           , edm::EventSetup const & /*unused*/
                              )
{
  for (int i=0; i<10000000; ++i) {
       LogDebug    ("dog") << "I am perhaps creating a long string here";
  }
  edm::LogInfo     ("cat") << "Event " << e.id() << "complete";
}  // MessageLoggerClient::analyze()

}  // namespace edmtest


using edmtest::UnitTestClient_L;
using edmtest::UnitTestClient_L1;
DEFINE_FWK_MODULE(UnitTestClient_L);
DEFINE_FWK_MODULE(UnitTestClient_L1);
