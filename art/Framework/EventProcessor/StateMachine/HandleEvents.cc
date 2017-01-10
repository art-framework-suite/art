#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleEvents::HandleEvents(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()},
    currentEvent_{ep_.eventPrincipalID()}
  {}

  void HandleEvents::disableFinalizeEvent(Pause const&)
  {
    context<HandleFiles>().disallowStaging();
    finalizeEnabled_ = false;
  }

  void HandleEvents::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    exitCalled_ = true;
    finalizeEvent();
  }

  HandleEvents::~HandleEvents()
  {
    if (!exitCalled_) {
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
    if (!finalizeEnabled_) return;
    if (eventException_) return;
    if (currentEvent_.isFlush()) return;

    eventException_ = true;
    context<HandleFiles>().openSomeOutputFiles();
    ep_.writeEvent();

    // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
    // is called due to exiting a 'Pause' state.
    if (context<HandleFiles>().stagingAllowed()) {
      ep_.recordOutputClosureRequests(Boundary::Event);
      context<HandleFiles>().maybeTriggerOutputFileSwitch();
    }

    eventException_ = false;
  }

  NewEvent::NewEvent(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    auto& handleEvents = context<HandleEvents>();
    handleEvents.setEventException(true);
    markNonEmpty();
    handleEvents.setCurrentEvent(ep_.readEvent());
    handleEvents.setEventException(false);
    checkInvariant();
    post_event(Process{});
  }

  NewEvent::~NewEvent()
  {
    checkInvariant();
  }

  void NewEvent::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleRuns>().beginRunCalled());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
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
    if (context<HandleEvents>().currentEvent().isFlush()) return;

    ep_.processEvent();
    if (ep_.shouldWeStop()) {
      post_event(Stop{});
    }
  }

  PauseEvent::PauseEvent(my_context ctx)
    : my_base{ctx}
  {}

}
