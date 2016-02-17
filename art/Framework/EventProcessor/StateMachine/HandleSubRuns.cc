#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleSubRuns::HandleSubRuns(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()},
    currentSubRun_{ep_.subRunPrincipalID()}
  {
    // std::cout << " HandleSubRuns()\n";
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
    // std::cout << "~HandleSubRuns()\n";
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

  void HandleSubRuns::resumeAndFinalizeSubRun(SubRun const&)
  {
    resetFormerState();
    finalizeSubRun();
  }

  void HandleSubRuns::resume(Event const&) { resetFormerState(); }

  void HandleSubRuns::resetFormerState()
  {
    finalizeEnabled_ = true;
  }

  void HandleSubRuns::beginSubRun(art::SubRunID sr)
  {
    beginSubRunCalled_ = true;
    subRunException_ = true;
    ep_.beginSubRun(sr);
    subRunException_ = false;
  }

  void HandleSubRuns::endSubRun(art::SubRunID sr)
  {
    beginSubRunCalled_ = false;
    subRunException_ = true;
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

    subRunException_ = true;
    if (context<Machine>().handleEmptySubRuns()) {
      context<HandleRuns>().beginRunIfNotDoneAlready();
      if (!beginSubRunCalled_) beginSubRun(currentSubRun());
    }
    if (beginSubRunCalled_) endSubRun(currentSubRun());
    ep_.writeSubRun(currentSubRun_);
    ep_.recordOutputClosureRequests();
    context<HandleFiles>().maybeTriggerOutputFileSwitch(Boundary::SubRun);
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
    // std::cout << " NewSubRun()\n";
    context<HandleFiles>().setCurrentBoundary(Boundary::SubRun);
    context<HandleSubRuns>().setupCurrentSubRun();
    checkInvariant();
  }

  NewSubRun::~NewSubRun()
  {
    // std::cout << "~NewSubRun()\n";
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
  {
    // std::cout << " PauseSubRun()\n";
  }

  PauseSubRun::~PauseSubRun()
  {
    // std::cout << "~PauseSubRun()\n";
  }

}
