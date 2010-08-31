#include "test/Framework/Services/Message/UnitTestClient_D.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>
#include <iomanip>

namespace edmtest
{


void
  UnitTestClient_D::analyze( edm::Event      const & e
                           , edm::EventSetup const & /*unused*/
                              )
{
  edm::LogWarning("cat_A")   << "This message should not appear in "
  			     << "the framework job report";
  edm::LogWarning("FwkJob")  << "<Message>This message should appear in "
 			     << "the framework job report</Message>";
  edm::LogWarning("special") << "This message should appear in "
 			     << "restrict but the others should not";

}  // MessageLoggerClient::analyze()


}  // namespace edmtest


using edmtest::UnitTestClient_D;
DEFINE_FWK_MODULE(UnitTestClient_D);
