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
  {
  }

  void HandleEvents::disableProcessAndFinalizeEvent(Pause const&)
  {
    context<HandleFiles>().disallowStaging();
    processAndFinalizeEnabled_ = false;
  }

  void HandleEvents::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    exitCalled_ = true;
    processAndFinalizeEvent();
  }

  HandleEvents::~HandleEvents()
  {
    if (!exitCalled_) {
      try {
        processAndFinalizeEvent();
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
                << "std::bad_alloc exception thrown inside HandleEvents::processAndFinalizeEvent.\n"
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
                << "standard library exception thrown inside HandleEvents::processAndFinalizeEvent\n"
                << e.what() << "\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleEvents::processAndFinalizeEvent\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
    }
  }

  void HandleEvents::processAndFinalizeEvent()
  {
    if (!processAndFinalizeEnabled_) return;
    if (eventException_) return;

    eventException_ = true;
    ep_.processEvent();
    if (ep_.shouldWeStop()) { post_event(Stop()); }
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
    ep_.readEvent();
    handleEvents.setEventException(false);
    checkInvariant();
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

  PauseEvent::PauseEvent(my_context ctx)
    : my_base(ctx)
  {
  }

}
