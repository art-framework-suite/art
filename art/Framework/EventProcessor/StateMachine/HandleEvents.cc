#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleEvents::HandleEvents(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()},
    currentEvent_{ep_.eventPrincipalID()}
  {
    // std::cout << " HandleEvents()\n";
  }

  void HandleEvents::checkInvariant()
  {
    assert(true);
  }

  void HandleEvents::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    exitCalled_ = true;
    checkInvariant();
    finalizeEvent();
  }

  HandleEvents::~HandleEvents()
  {
    // std::cout << "~HandleEvents()\n";
    if (!exitCalled_) {
      try {
        checkInvariant();
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
    context<HandleFiles>().openSomeOutputFiles();
    ep_.writeEvent();
    ep_.recordOutputClosureRequests();
    context<HandleFiles>().maybeTriggerOutputFileSwitch(Boundary::Event);
  }

  NewEvent::NewEvent(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    // std::cout << " NewEvent()\n";
    context<Machine>().setCurrentBoundary(Boundary::Event);
    readAndProcessEvent();
    checkInvariant();
  }

  NewEvent::~NewEvent()
  {
    // std::cout << "~NewEvent()\n";
    checkInvariant();
  }

  void NewEvent::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleRuns>().beginRunCalled());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
  }

  void NewEvent::readAndProcessEvent()
  {
    markNonEmpty();
    ep_.readEvent();
    ep_.processEvent();
    if (ep_.shouldWeStop()) { post_event(Stop()); }
  }

  void NewEvent::markNonEmpty()
  {
    context<HandleSubRuns>().markSubRunNonEmpty();
    context<HandleSubRuns>().beginSubRunIfNotDoneAlready();
  }

  PauseEvent::PauseEvent(my_context ctx)
    : my_base(ctx)
  {
    // std::cout << " PauseEvent()\n";
  }

  PauseEvent::~PauseEvent()
  {
    // std::cout << "~PauseEvent()\n";
  }

}
