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
    ep_.setCurrentEvent(ep_.eventPrincipalID());
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
    finalizeEvent();
  }

  HandleEvents::~HandleEvents()
  {
    if (!ep_.exitEventCalled()) {
      try {
        finalizeEvent();
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

  void HandleEvents::finalizeEvent()
  {
    if (!ep_.finalizeEventEnabled()) return;
    if (ep_.eventException()) return;
    if (ep_.currentEvent().isFlush()) return;

    ep_.setEventException(true);
    ep_.openSomeOutputFiles();
    ep_.writeEvent();

    // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
    // is called due to exiting a 'Pause' state.
    if (ep_.stagingAllowed()) {
      ep_.recordOutputClosureRequests(Boundary::Event);
      ep_.maybeTriggerOutputFileSwitch();
    }

    ep_.setEventException(false);
  }

  NewEvent::NewEvent(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.setEventException(true);
    markNonEmpty();
    ep_.setCurrentEvent(ep_.readEvent());
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
    assert(ep_.currentRun().isValid());
    assert(ep_.beginRunCalled());
    assert(ep_.currentSubRun().runID() == ep_.currentRun());
    assert(ep_.currentSubRun().isValid());
  }

  void NewEvent::markNonEmpty()
  {
    context<HandleSubRuns>().markSubRunNonEmpty();
    context<HandleSubRuns>().beginSubRunIfNotDoneAlready();
  }

  ProcessEvent::ProcessEvent(my_context ctx)
    : my_base{ctx}
    , ep_{context<Machine>().ep()}
  {
    if (ep_.currentEvent().isFlush()) return;

    ep_.processEvent();
    if (ep_.shouldWeStop()) {
      post_event(Stop{});
    }
  }

  PauseEvent::PauseEvent(my_context ctx)
    : my_base{ctx}
  {}

}
