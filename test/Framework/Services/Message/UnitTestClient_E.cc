#include "test/Framework/Services/Message/UnitTestClient_E.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_E::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
    mf::LogInfo   ("expect_overall_unnamed")
    << "The following outputs are expected: \n"
    << "unlisted_category    appearing in events 1,2,3,4,5,10,15,25,45 \n";

    mf::LogInfo   ("expect_overall_specific")
    << "The following outputs are expected: \n"
    << "lim3bydefault    appearing in events 1,2,3,6,9,15,27 \n";

    mf::LogInfo   ("expect_supercede_specific")
    << "The following outputs are expected: \n"
    << "lim2bycommondefault appearing in events 1,2,3,4,5,6,7,8,16,24,40 \n";

    mf::LogInfo   ("expect_non_supercede_common_specific")
    << "The following outputs are expected: \n"
    << "lim2bycommondefault appearing in events 1,2,4,6,10,18,34 \n";

    mf::LogInfo   ("expect_specific")
    << "The following outputs are expected: \n"
    << "lim0bydefaults appearing in events 1,2,3,4,5,6,12,18,30 \n";

  for (int i=1; i<=50; ++i) {
    mf::LogInfo   ("unlisted_category")   <<
  	"message with overall default limit of 5: " << i;
    mf::LogInfo   ("lim3bydefault")   <<
  	"message with specific overall default limit of 3: " << i;
    mf::LogInfo   ("lim2bycommondefault")   <<
  	"message with specific overall default limit of 2: " << i;
    mf::LogInfo   ("lim0bydefaults")   <<
  	"message with overall default and dest default limit of 0: " << i;
//    mf::LogInfo   ("lim3bydefault")   <<
//  	"message with overall default limit (superceded) of 2: " << i;
   }

}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_E;
DEFINE_ART_MODULE(UnitTestClient_E);
