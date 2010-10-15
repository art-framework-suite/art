#include "test/Framework/Services/Message/UnitTestClient_F.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  UnitTestClient_F::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
    art::LogInfo   ("expect_overall_unnamed")
    << "The following outputs are expected: \n"
    << "unlisted_category    appearing in events 1,6,11,16,21,26,31,36,41,46 \n";

    art::LogInfo   ("expect_overall_specific")
    << "The following outputs are expected: \n"
    << "int7bycommondefault    appearing in events 1,8,15,22,29,36,43,50 \n";

    art::LogInfo   ("expect_supercede_specific")
    << "The following outputs are expected: \n"
    << "int7bycommondefault appearing in events 1,11,21,31,41 \n";

    art::LogInfo   ("expect_non_supercede_common_specific")
    << "The following outputs are expected: \n"
    << "int7bycommondefault appearing in events 1,19,37 \n"
    << "unlisted_category appearing in events 1,27 \n";

    art::LogInfo   ("expect_specific")
    << "The following outputs are expected: \n"
    << "int25bydefaults appearing in events 1,13,25,37,49 \n"
    << "unlisted_category appearing in events 1,31 \n";

  for (int i=1; i<=50; ++i) {
    art::LogInfo   ("unlisted_category")   <<
  	"message with overall default interval of 5: " << i;
    art::LogInfo   ("int4bydefault")   <<
  	"message with specific overall default interval of 4: " << i;
    art::LogInfo   ("int7bycommondefault")   <<
  	"message with specific overall default interval of 7: " << i;
    art::LogInfo   ("int25bydefaults")   <<
  	"message with overall default and dest default interval of 25: " << i;
   }

}  // MessageLoggerClient::analyze()


}  // namespace arttest


using arttest::UnitTestClient_F;
DEFINE_FWK_MODULE(UnitTestClient_F);
