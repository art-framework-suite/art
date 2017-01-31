#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleRuns::HandleRuns(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.setBeginRunCalled(false);
    ep_.setExitRunCalled(false);
    ep_.setRunException(false);
    ep_.setFinalizeRunEnabled(true);
  }

  void HandleRuns::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    ep_.setExitRunCalled(true);
    ep_.finalizeRun();
  }

  HandleRuns::~HandleRuns()
  {
    if (!ep_.exitRunCalled()) {
      try {
        ep_.finalizeRun();
      }
      catch (cet::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs after\n"
                << "the primary exception.  We give up trying to clean up runs at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        ep_.setExceptionMessageRuns(message.str());
      }
      catch (std::bad_alloc const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs\n"
                << "after the primary exception.  We give up trying to clean up runs\n"
                << "at this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleRuns::finalizeRun.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        ep_.setExceptionMessageRuns(message.str());
      }
      catch (std::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs after\n"
                << "the primary exception.  We give up trying to clean up runs at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleRuns::finalizeRun\n"
                << e.what() << "\n";
        ep_.setExceptionMessageRuns(message.str());
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs after\n"
                << "the primary exception.  We give up trying to clean up runs at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleRuns::finalizeRun\n";
        ep_.setExceptionMessageRuns(message.str());
      }
    }
  }

  void HandleRuns::disableFinalizeRun(Pause const&)
  {
    ep_.disallowStaging();
    ep_.setFinalizeRunEnabled(false);
  }

  NewRun::NewRun(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.setupCurrentRun();
    // Here we assume that the input source or event processor
    // will throw if we fail to get a valid run.  Therefore
    // we should not ever fail this assert.
    assert(ep_.runPrincipalID().isValid());
  }

  PauseRun::PauseRun(my_context ctx)
    : my_base{ctx}
  {}

}
