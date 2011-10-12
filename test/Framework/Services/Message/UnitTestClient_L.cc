#include "test/Framework/Services/Message/UnitTestClient_L.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

#include <iostream>
#include <string>


namespace arttest
{


void UnitTestClient_L::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  for (int i=0; i<10000000; ++i) {
  }
  mf::LogInfo     ("cat") << "Event " << e.id() << "complete";
}  // MessageLoggerClient::analyze()

void UnitTestClient_L1::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  for (int i=0; i<10000000; ++i) {
       LogDebug    ("dog") << "I am perhaps creating a long string here";
  }
  mf::LogInfo     ("cat") << "Event " << e.id() << "complete";
}  // MessageLoggerClient::analyze()

}  // arttest


using arttest::UnitTestClient_L;
using arttest::UnitTestClient_L1;
DEFINE_ART_MODULE(UnitTestClient_L);
DEFINE_ART_MODULE(UnitTestClient_L1);
