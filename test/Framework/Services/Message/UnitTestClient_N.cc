#include "test/Framework/Services/Message/UnitTestClient_N.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/ModuleMacros.h"

#include <iostream>
#include <string>

namespace arttest {


  void
  UnitTestClient_N::analyze(art::Event      const & e
                            , art::EventSetup const & /*unused*/
                           )
  {
    std::string empty_;
    std::string file_ = "nameOfFile";
    LogDebug("ridiculously_long_category_name_to_make_header_wrap_A")
        << "LogDebug was used to send this message";
    LogDebug("ridiculously_long_category_name_to_make_header_wrap_B")
        << "LogDebug was used to send this other message";
    mf::LogInfo("ridiculously_long_category_name_to_make_header_wrap_A")
        << "LogInfo was used to send this message";
    mf::LogInfo("ridiculously_long_category_name_to_make_header_wrap_B")
        << "LogInfo was used to send this other message";
  }  // MessageLoggerClient::analyze()


}  // arttest


using arttest::UnitTestClient_N;
DEFINE_ART_MODULE(UnitTestClient_N);
