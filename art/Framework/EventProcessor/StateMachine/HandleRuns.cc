#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleRuns::HandleRuns(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()},
    currentRun_{ep_.runPrincipalID()}
  {
    // std::cout << " HandleRuns()\n";
  }

  void HandleRuns::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    exitCalled_ = true;
    finalizeRun();
  }

  HandleRuns::~HandleRuns()
  {
    // std::cout << "~HandleRuns()\n";
    if (!exitCalled_) {
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

  bool HandleRuns::beginRunCalled() const { return beginRunCalled_; }
  art::RunID HandleRuns::currentRun() const { return currentRun_; }

  void HandleRuns::setupCurrentRun()
  {
    runException_ = true;
    currentRun_ = ep_.readAndCacheRun();
    runException_ = false;
    if (context<Machine>().handleEmptyRuns()) {
      beginRun(currentRun());
    }
  }

  void HandleRuns::beginRun(art::RunID run)
  {
    beginRunCalled_ = true;
    runException_ = true;
    ep_.beginRun(run);
    runException_ = false;
  }

  void HandleRuns::endRun(art::RunID run)
  {
    beginRunCalled_ = false;
    runException_ = true;
    ep_.endRun(run);
    runException_ = false;
  }

  void HandleRuns::finalizeRun(Run const &)
  {
    finalizeRun();
  }

  void HandleRuns::finalizeRun()
  {
    if (!finalizeEnabled_) return;
    if (runException_) return;

    runException_ = true;
    context<HandleFiles>().openSomeOutputFiles();
    if (beginRunCalled_) endRun(currentRun());
    ep_.writeRun(currentRun_);
    ep_.recordOutputClosureRequests();
    context<HandleFiles>().maybeTriggerOutputFileSwitch(Boundary::Run);
    currentRun_ = art::RunID(); // Invalid.
    runException_ = false;
  }

  void HandleRuns::beginRunIfNotDoneAlready()
  {
    if (!beginRunCalled_) beginRun(currentRun());
  }

  NewRun::NewRun(my_context ctx) :
    my_base{ctx}
  {
    // std::cout << " NewRun()\n";
    context<Machine>().setCurrentBoundary(Boundary::Run);
    context<HandleRuns>().setupCurrentRun();
    // Here we assume that the input source or event processor
    // will throw if we fail to get a valid run.  Therefore
    // we should not ever fail this assert.
    assert(context<HandleRuns>().currentRun().isValid());
  }

  NewRun::~NewRun() {
    // std::cout << "~NewRun()\n";
  }

  PauseRun::PauseRun(my_context ctx)
    : my_base{ctx}
  {
    // std::cout << " PauseRun()\n";
  }

  PauseRun::~PauseRun()
  {
    // std::cout << "~PauseRun()\n";
  }

}
