// ----------------------------------------------------------------------
//
// SingleThreadMSPresence.cc
//
// Changes:
//
//

#include "art/Framework/Services/Message/SingleThreadMSPresence.h"
#include "art/Framework/Services/Message/MessageLoggerScribe.h"

#include "art/MessageLogger/MessageLoggerQ.h"
#include "art/MessageLogger/MessageDrop.h"


namespace edm {
namespace service {


SingleThreadMSPresence::SingleThreadMSPresence()
  : Presence()
  , m(true)
{
  //std::cout << "SingleThreadMSPresence ctor\n";
  MessageLoggerQ::setMLscribe_ptr(&m);
  MessageDrop::instance()->messageLoggerScribeIsRunning =
  				MLSCRIBE_RUNNING_INDICATOR;
}


SingleThreadMSPresence::~SingleThreadMSPresence()
{
  MessageLoggerQ::MLqEND();

}

} // end of namespace service
} // end of namespace edm
