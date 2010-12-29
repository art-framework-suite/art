#include "test/Framework/Services/Message/makeSignals.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/MessageLogger/MessageDrop.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"

#include <iostream>
#include <string>
#include <csignal>

#define RAISE_SEGV
//#define RAISE_USR2

namespace arttest
{

void
  makeSignals::analyze( art::Event      const & e
                       ,art::EventSetup const & /*unused*/
                      )
{
#ifdef RAISE_SEGV
  int signum = 11;
  std::string SigName("SIGSEGV");
#endif

#ifdef RAISE_USR2
  int signum = 12;
  std::string SigName("SIGUSR2");
#endif
  art::MessageDrop::instance()->debugEnabled  = true;

       LogTrace    ("cat_A") << "LogTrace was used to send this mess" << "age";
       LogDebug    ("cat_B") << "LogDebug was used to send this other message";
  art::LogVerbatim ("cat_A") << "LogVerbatim was us" << "ed to send this message";
  if( art::isInfoEnabled() )
     art::LogInfo  ("cat_B") << "LogInfo was used to send this other message\n" ;

  if( e.id().event() == 5 )
   {
    std::cerr << "Raising Signal " << SigName << " = " << signum << std::endl;
    art::LogInfo("Signals") << "Raising Signal " << SigName << " = " << signum ;
#ifdef RAISE_SEGV
    raise(SIGSEGV);
#endif

#ifdef RAISE_USR2
    raise(SIGUSR2);
#endif

//  Force a Seg Fault
//  int * pint = 0;
//  int rint = *pint;
   }
}  // makeSignals::analyze()
}  // arttest


using arttest::makeSignals;
DEFINE_FWK_MODULE(makeSignals);
