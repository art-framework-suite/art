#include "art/test/Framework/Services/Message/UnitTestClient_G.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>
#include <iomanip>

namespace arttest
{


void
  UnitTestClient_G::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  if (!art::isMessageProcessingSetUp()) {
    std::cerr << "??? It appears that Message Processing is not Set Up???\n\n";
  }

  double d = 3.14159265357989;
  mf::LogWarning("cat_A")   << "Test of std::setprecision(p):"
                             << " Pi with precision 12 is "
                             << std::setprecision(12) << d;

  for( int i=0; i<10; ++i) {
    mf::LogInfo("cat_B")      << "\t\tEmit Info level message " << i+1;
  }

  for( int i=0; i<15; ++i) {
    mf::LogWarning("cat_C")      << "\t\tEmit Warning level message " << i+1;
  }
}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_G;
DEFINE_ART_MODULE(UnitTestClient_G)
