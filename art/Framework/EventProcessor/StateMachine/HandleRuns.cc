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
      ep_.try_finalize<art::Level::Run>();
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
