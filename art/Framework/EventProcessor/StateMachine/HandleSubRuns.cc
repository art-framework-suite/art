#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "cetlib/exception.h"

#include <exception>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleSubRuns::HandleSubRuns(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.setCurrentSubRun(ep_.subRunPrincipalID());
    ep_.setExitSubRunCalled(false);
    ep_.setBeginSubRunCalled(false);
    ep_.setSubRunException(false);
    ep_.setFinalizeSubRunEnabled(true);
    checkInvariant();
  }

  void HandleSubRuns::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    ep_.setExitSubRunCalled(true);
    checkInvariant();
    finalizeSubRun();
  }

  HandleSubRuns::~HandleSubRuns()
  {
    if (!ep_.exitSubRunCalled()) {
      try {
        checkInvariant();
        finalizeSubRun();
      }
      catch (cet::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::bad_alloc const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns\n"
                << "after the primary exception.  We give up trying to clean up subRuns\n"
                << "at this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleSubRuns::finalizeAllSubRuns.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleSubRuns::finalizeAllSubRuns\n"
                << e.what() << "\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleSubRuns::finalizeAllSubRuns\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
    }
  }

  void HandleSubRuns::checkInvariant()
  {
    assert(ep_.currentRun().isValid());
  }

  void HandleSubRuns::setupCurrentSubRun()
  {
    assert(ep_.currentRun().isValid());
    ep_.setSubRunException(true);
    ep_.setCurrentSubRun(ep_.readSubRun());
    ep_.setSubRunException(false);
  }

  void HandleSubRuns::disableFinalizeSubRun(Pause const&)
  {
    ep_.disallowStaging();
    ep_.setFinalizeSubRunEnabled(false);
  }
  void HandleSubRuns::beginSubRun()
  {
    ep_.setBeginSubRunCalled(true);
    ep_.setSubRunException(true);
    if (!ep_.currentSubRun().isFlush())
      ep_.beginSubRun();
    ep_.setSubRunException(false);
  }

  void HandleSubRuns::endSubRun()
  {
    ep_.setBeginSubRunCalled(false);
    ep_.setSubRunException(true);
    // Note: flush flag is not checked here since endSubRun is only
    // called from finalizeRun, which is where the check happens.
    ep_.endSubRun();
    ep_.setSubRunException(false);
  }

  void HandleSubRuns::finalizeSubRun()
  {
    if (!ep_.finalizeSubRunEnabled()) return;
    if (ep_.subRunException()) return;
    if (ep_.currentSubRun().isFlush()) return;

    ep_.setSubRunException(true);
    ep_.openSomeOutputFiles();
    if (ep_.handleEmptySubRuns()) {
      context<HandleRuns>().beginRunIfNotDoneAlready();
      beginSubRunIfNotDoneAlready();
    }
    ep_.setSubRunAuxiliaryRangeSetID();
    if (ep_.beginSubRunCalled())
      endSubRun();
    ep_.writeSubRun();

    // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
    // is called due to exiting a 'Pause' state.
    if (ep_.stagingAllowed()) {
      ep_.recordOutputClosureRequests(Boundary::SubRun);
      ep_.maybeTriggerOutputFileSwitch();
    }

    ep_.setCurrentSubRun(art::SubRunID{}); // Invalid.
    ep_.setSubRunException(false);
  }

  void HandleSubRuns::beginSubRunIfNotDoneAlready()
  {
    if (!ep_.beginSubRunCalled()) beginSubRun();
  }

  void HandleSubRuns::markSubRunNonEmpty()
  {
    context<HandleRuns>().beginRunIfNotDoneAlready();
  }

  NewSubRun::NewSubRun(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    context<HandleSubRuns>().setupCurrentSubRun();
    checkInvariant();
  }

  NewSubRun::~NewSubRun()
  {
    checkInvariant();
  }

  void NewSubRun::checkInvariant()
  {
    assert(ep_.currentRun().isValid());
    assert(ep_.currentSubRun().runID() == ep_.currentRun());
    assert(ep_.currentSubRun().isValid());
  }

  PauseSubRun::PauseSubRun(my_context ctx)
    : my_base{ctx}
  {}

}
