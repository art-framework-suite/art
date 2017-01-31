#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "cetlib/exception.h"

#include <exception>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleEvents::HandleEvents(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.setExitEventCalled(false);
    ep_.setEventException(false);
    ep_.setFinalizeEventEnabled(true);
  }

  void HandleEvents::disableFinalizeEvent(Pause const&)
  {
    ep_.disallowStaging();
    ep_.setFinalizeEventEnabled(false);
  }

  void HandleEvents::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    ep_.setExitEventCalled(true);
    ep_.finalizeEvent();
  }

  HandleEvents::~HandleEvents()
  {
    if (!ep_.exitEventCalled()) {
      try {
        ep_.finalizeEvent();
      }
      catch (cet::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::bad_alloc const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleEvents::finalizeEvent.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleEvents::finalizeEvent\n"
                << e.what() << "\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleEvents::finalizeEvent\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
    }
  }

  NewEvent::NewEvent(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.setEventException(true);
    // If we got here, the Run and SubRun are non empty.  Call
    // appropriate beginRun/beginSubRun functions.
    ep_.beginRunIfNotDoneAlready();
    ep_.beginSubRunIfNotDoneAlready();
    ep_.readEvent();
    ep_.setEventException(false);
    checkInvariant();
    post_event(Process{});
  }

  NewEvent::~NewEvent()
  {
    checkInvariant();
  }

  void NewEvent::checkInvariant()
  {
    assert(ep_.runPrincipalID().isValid());
    assert(ep_.beginRunCalled());
    assert(ep_.subRunPrincipalID().runID() == ep_.runPrincipalID());
    assert(ep_.subRunPrincipalID().isValid());
  }

  ProcessEvent::ProcessEvent(my_context ctx)
    : my_base{ctx}
    , ep_{context<Machine>().ep()}
  {
    if (ep_.eventPrincipalID().isFlush()) return;

    ep_.processEvent();
    if (ep_.shouldWeStop()) {
      post_event(Stop{});
    }
  }

  PauseEvent::PauseEvent(my_context ctx)
    : my_base{ctx}
  {}

}
