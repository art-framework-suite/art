#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleSubRuns::HandleSubRuns(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()},
    currentSubRun_{ep_.subRunPrincipalID()}
  {
    checkInvariant();
  }

  void HandleSubRuns::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    exitCalled_ = true;
    checkInvariant();
    finalizeSubRun();
  }

  HandleSubRuns::~HandleSubRuns()
  {
    if (!exitCalled_) {
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
    assert(context<HandleRuns>().currentRun().isValid());
  }

  bool HandleSubRuns::beginSubRunCalled() const { return beginSubRunCalled_; }
  art::SubRunID const& HandleSubRuns::currentSubRun() const { return currentSubRun_; }

  void HandleSubRuns::setupCurrentSubRun()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    subRunException_ = true;
    currentSubRun_ = ep_.readAndCacheSubRun();
    subRunException_ = false;
  }

  void HandleSubRuns::disableFinalizeSubRun(Pause const&)
  {
    context<HandleFiles>().disallowStaging();
    finalizeEnabled_ = false;
  }
  void HandleSubRuns::beginSubRun(art::SubRunID sr)
  {
    beginSubRunCalled_ = true;
    subRunException_ = true;
    if (!sr.isFlush())
      ep_.beginSubRun(sr);
    subRunException_ = false;
  }

  void HandleSubRuns::endSubRun(art::SubRunID sr)
  {
    beginSubRunCalled_ = false;
    subRunException_ = true;
    // Note: flush flag is not checked here since endSubRun is only
    // called from finalizeRun, which is where the check happens.
    ep_.endSubRun(sr);
    subRunException_ = false;
  }

  void HandleSubRuns::finalizeSubRun(SubRun const&)
  {
    finalizeSubRun();
  }

  void HandleSubRuns::finalizeSubRun()
  {
    if (!finalizeEnabled_) return;
    if (subRunException_) return;
    if (currentSubRun_.isFlush()) return;

    subRunException_ = true;
    context<HandleFiles>().openSomeOutputFiles();
    if (context<Machine>().handleEmptySubRuns()) {
      context<HandleRuns>().beginRunIfNotDoneAlready();
      beginSubRunIfNotDoneAlready();
    }
    ep_.setSubRunAuxiliaryRangeSetID(currentSubRun_);
    if (beginSubRunCalled_) endSubRun(currentSubRun());
    ep_.writeSubRun(currentSubRun_);

    // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
    // is called due to exiting a 'Pause' state.
    if (context<HandleFiles>().stagingAllowed()) {
      ep_.recordOutputClosureRequests(Boundary::SubRun);
      context<HandleFiles>().maybeTriggerOutputFileSwitch();
    }

    currentSubRun_ = art::SubRunID(); // Invalid.
    subRunException_ = false;
  }

  void HandleSubRuns::beginSubRunIfNotDoneAlready()
  {
    if (!beginSubRunCalled_) beginSubRun(currentSubRun());
  }

  void HandleSubRuns::markSubRunNonEmpty()
  {
    context<HandleRuns>().beginRunIfNotDoneAlready();
  }

  NewSubRun::NewSubRun(my_context ctx) :
    my_base{ctx}
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
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
  }

  PauseSubRun::PauseSubRun(my_context ctx)
    : my_base{ctx}
  {}

}
