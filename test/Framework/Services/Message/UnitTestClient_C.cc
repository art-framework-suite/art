#include "test/Framework/Services/Message/UnitTestClient_C.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>
#include <iomanip>

namespace arttest
{


void
  UnitTestClient_C::analyze( art::Event      const & e
                           , art::EventSetup const & /*unused*/
                              )
{
  int i = 145;
  art::LogWarning("cat_A")   << "Test of std::hex:"
  			     << i << std::hex << "in hex is"  << i;
   art::LogWarning("cat_A")  << "Test of std::setw(n) and std::setfill('c'):"
   			     << "The following should read ++abcdefg $$$12:"
  			     << std::setfill('+')  << std::setw(9) << "abcdefg"
			     << std::setw(5) << std::setfill('$') << 12 ;
  double d = 3.14159265357989;
  art::LogWarning("cat_A")   << "Test of std::setprecision(p):"
  			     << "Pi with precision 12 is"
  			     << std::setprecision(12) << d;
  art::LogWarning("cat_A")   << "Test of spacing:"
   			     << "The following should read a b c dd:"
			     << "a" <<  std::setfill('+')
			     << "b" << std::hex << "c" << std::setw(2) << "dd";
}  // MessageLoggerClient::analyze()


}  // namespace arttest


using arttest::UnitTestClient_C;
DEFINE_FWK_MODULE(UnitTestClient_C);
