#include "test/Framework/Services/Message/ProblemTestClient_t1.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>

namespace arttest
{


void
  ProblemTestClient_t1::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
       LogDebug  ("cat_A")   << "This message should not appear";
       LogDebug  ("TrackerGeom")  << "LogDebug was used to send this message";

}  // MessageLoggerClient::analyze()


}  // arttest


using arttest::ProblemTestClient_t1;
DEFINE_FWK_MODULE(ProblemTestClient_t1);
