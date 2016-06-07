#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  Machine::Machine(art::IEventProcessor * ep,
                   bool handleEmptyRuns,
                   bool handleEmptySubRuns) :
    ep_{ep},
    handleEmptyRuns_{handleEmptyRuns},
    handleEmptySubRuns_{handleEmptySubRuns} { }

  art::IEventProcessor & Machine::ep() const { return *ep_; }
  bool Machine::handleEmptyRuns() const { return handleEmptyRuns_; }
  bool Machine::handleEmptySubRuns() const { return handleEmptySubRuns_; }

  void Machine::goToNewInputFile(InputFile const&)
  {
    setCurrentBoundary(Boundary::InputFile);
    closeSomeOutputFiles();
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
    ep_->respondToCloseOutputFiles();
    // If the previous state was (e.g.) NewSubRun, and the current
    // state is NewEvent, and an output file needs to close, then it
    // should be allowed to.  Setting 'start' equal to
    // 'previousBoundary_' would result in the for-loop below not
    // being executed.  We therefore take the minimum of the
    // boundaries as the starting point, although the end point is
    // always the current boundary.
    auto const start = std::min(previousBoundary_, currentBoundary_);
    for(std::size_t b = start; b<=currentBoundary_; ++b) {
      ep_->closeSomeOutputFiles(b);
    }
  }

  void Machine::closeSomeOutputFiles(SwitchOutputFiles const&)
  {
    closeSomeOutputFiles();
  }

  void Machine::setCurrentBoundary(Boundary::BT const b)
  {
    previousBoundary_ = currentBoundary_;
    currentBoundary_ = b;
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

}
