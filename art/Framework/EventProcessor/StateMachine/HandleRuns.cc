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
    ep_.setCurrentRun(ep_.runPrincipalID());
  }

  void HandleRuns::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    ep_.setExitRunCalled(true);
    finalizeRun();
  }

  HandleRuns::~HandleRuns()
  {
    if (!ep_.exitRunCalled()) {
      try {
        finalizeRun();
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

  void HandleRuns::setupCurrentRun()
  {
    ep_.setRunException(true);
    ep_.setCurrentRun(ep_.readRun());
    ep_.setRunException(false);
    if (ep_.handleEmptyRuns()) {
      beginRun();
    }
  }

  void HandleRuns::disableFinalizeRun(Pause const&)
  {
    ep_.disallowStaging();
    ep_.setFinalizeRunEnabled(false);
  }

  void HandleRuns::beginRun()
  {
    ep_.setBeginRunCalled(true);
    ep_.setRunException(true);
    if (!ep_.currentRun().isFlush())
      ep_.beginRun();
    ep_.setRunException(false);
  }

  void HandleRuns::endRun()
  {
    ep_.setRunException(true);
    // Note: flush flag is not checked here since endRun is only
    // called from finalizeRun, which is where the check happens.
    ep_.endRun();
    ep_.setRunException(false);
  }

  void HandleRuns::finalizeRun()
  {
    if (!ep_.finalizeRunEnabled()) return;
    if (ep_.runException()) return;
    if (ep_.currentRun().isFlush()) return;

    ep_.setRunException(true);
    ep_.openSomeOutputFiles();
    ep_.setRunAuxiliaryRangeSetID();
    if (ep_.beginRunCalled())
      endRun();
    ep_.writeRun();

    // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
    // is called due to exiting a 'Pause' state.
    if (ep_.stagingAllowed()) {
      ep_.recordOutputClosureRequests(Boundary::Run);
      ep_.maybeTriggerOutputFileSwitch();
    }

    ep_.setCurrentRun(art::RunID{}); // Invalid.
    ep_.setRunException(false);
  }

  void HandleRuns::beginRunIfNotDoneAlready()
  {
    if (!ep_.beginRunCalled())
      beginRun();
  }

  NewRun::NewRun(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    context<HandleRuns>().setupCurrentRun();
    // Here we assume that the input source or event processor
    // will throw if we fail to get a valid run.  Therefore
    // we should not ever fail this assert.
    assert(ep_.currentRun().isValid());
  }

  PauseRun::PauseRun(my_context ctx)
    : my_base{ctx}
  {}

}
