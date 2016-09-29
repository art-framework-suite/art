#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  Machine::Machine(art::IEventProcessor* ep,
                   bool handleEmptyRuns,
                   bool handleEmptySubRuns) :
    ep_{ep},
    handleEmptyRuns_{handleEmptyRuns},
    handleEmptySubRuns_{handleEmptySubRuns}
  {}

  art::IEventProcessor& Machine::ep() const { return *ep_; }
  bool Machine::handleEmptyRuns() const { return handleEmptyRuns_; }
  bool Machine::handleEmptySubRuns() const { return handleEmptySubRuns_; }

  void Machine::closeInputFile(InputFile const&)
  {
    closeInputFile();
  }

  void Machine::closeAllOutputFiles()
  {
    if (!ep_->someOutputsOpen()) return;

    ep_->respondToCloseOutputFiles();
    ep_->closeAllOutputFiles();
  }

  void Machine::closeInputFile()
  {
    ep_->incrementInputFileNumber();
    ep_->recordOutputClosureRequests(Boundary::InputFile);
    if (ep_->outputsToClose()) {
      closeSomeOutputFiles();
    }
    ep_->respondToCloseInputFile();
    ep_->clearPrincipalCache();
    ep_->closeInputFile();
  }

  void Machine::closeAllFiles()
  {
    closeAllOutputFiles();
    closeInputFile();
  }

  void Machine::closeSomeOutputFiles()
  {
    // Precondition: there are SOME output files that have been
    //               flagged as needing to close.  Otherwise,
    //               'respondtoCloseOutputFiles' will be needlessly
    //               called.
    assert(ep_->outputsToClose());
    ep_->respondToCloseOutputFiles();
    ep_->closeSomeOutputFiles();
  }

  void Machine::closeSomeOutputFiles(SwitchOutputFiles const&)
  {
    closeSomeOutputFiles();
  }

  void Machine::closeAllFiles(Event const&) { closeAllFiles(); }
  void Machine::closeAllFiles(SubRun const&) { closeAllFiles(); }
  void Machine::closeAllFiles(Run const&) { closeAllFiles(); }
  void Machine::closeAllFiles(SwitchOutputFiles const&) { closeAllFiles(); }
  void Machine::closeAllFiles(Stop const&) { closeAllFiles(); }

  Starting::Starting(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.beginJob();
  }

    Stopping::Stopping(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.endJob();
    post_event(Stop{});
  }

  sc::result Stopping::react(Stop const &)
  {
    return terminate();
  }

  Error::Error(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    post_event(Stop{});
    ep_.doErrorStuff();
  }

}
