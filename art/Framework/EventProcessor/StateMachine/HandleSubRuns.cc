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
    ep_.finalizeSubRun();
  }

  HandleSubRuns::~HandleSubRuns()
  {
    if (!ep_.exitSubRunCalled()) {
      try {
        checkInvariant();
        ep_.finalizeSubRun();
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
    assert(ep_.runPrincipalID().isValid());
  }

  void HandleSubRuns::disableFinalizeSubRun(Pause const&)
  {
    ep_.disallowStaging();
    ep_.setFinalizeSubRunEnabled(false);
  }

  NewSubRun::NewSubRun(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.setupCurrentSubRun();
    checkInvariant();
  }

  NewSubRun::~NewSubRun()
  {
    checkInvariant();
  }

  void NewSubRun::checkInvariant()
  {
    assert(ep_.runPrincipalID().isValid());
    assert(ep_.subRunPrincipalID().runID() == ep_.runPrincipalID());
    assert(ep_.subRunPrincipalID().isValid());
  }

  PauseSubRun::PauseSubRun(my_context ctx)
    : my_base{ctx}
  {}

}
