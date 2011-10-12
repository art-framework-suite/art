#include "test/Framework/Services/Message/UnitTestClient_R.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>
#include <iomanip>

namespace arttest {


  void
  UnitTestClient_R::analyze(art::Event      const & e
                            , art::EventSetup const & /*unused*/
                           )
  {
    for (int i = 0; i < 10000; ++i) {
      mf::LogError("cat_A")   << "A " << i;
      mf::LogError("cat_B")   << "B " << i;
    }
  }  // MessageLoggerClient::analyze()

}  // arttest


using arttest::UnitTestClient_R;
DEFINE_ART_MODULE(UnitTestClient_R);
