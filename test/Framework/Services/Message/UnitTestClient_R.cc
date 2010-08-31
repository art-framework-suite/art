#include "test/Framework/Services/Message/UnitTestClient_R.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>
#include <iomanip>

namespace edmtest
{


void
  UnitTestClient_R::analyze( edm::Event      const & e
                           , edm::EventSetup const & /*unused*/
                              )
{

  for( int i=0; i<10000; ++i) {
    edm::LogError("cat_A")   << "A " << i;
    edm::LogError("cat_B")   << "B " << i;
  }
}  // MessageLoggerClient::analyze()

}  // namespace edmtest


using edmtest::UnitTestClient_R;
DEFINE_FWK_MODULE(UnitTestClient_R);
