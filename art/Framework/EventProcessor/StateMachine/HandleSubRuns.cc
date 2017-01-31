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
      checkInvariant();
      ep_.try_finalize<art::Level::SubRun>();
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
