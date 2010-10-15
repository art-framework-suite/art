#include "test/Framework/Services/Message/UnitTestClient_R.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>
#include <iomanip>

namespace arttest
{


void
  UnitTestClient_R::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{

  for( int i=0; i<10000; ++i) {
    art::LogError("cat_A")   << "A " << i;
    art::LogError("cat_B")   << "B " << i;
  }
}  // MessageLoggerClient::analyze()

}  // namespace arttest


using arttest::UnitTestClient_R;
DEFINE_FWK_MODULE(UnitTestClient_R);
