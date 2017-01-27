#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "cetlib/exception.h"

#include <exception>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  Machine::Machine(art::EventProcessor* ep) :
    ep_{ep}
  {}

  art::EventProcessor& Machine::ep() const { return *ep_; }

  void Machine::closeInputFile(InputFile const&)
  {
    ep_->closeInputFile();
  }

  void Machine::closeSomeOutputFiles(SwitchOutputFiles const&)
  {
    ep_->closeSomeOutputFiles();
  }

  void Machine::closeAllFiles(Event const&) { ep_->closeAllFiles(); }
  void Machine::closeAllFiles(SubRun const&) { ep_->closeAllFiles(); }
  void Machine::closeAllFiles(Run const&) { ep_->closeAllFiles(); }
  void Machine::closeAllFiles(SwitchOutputFiles const&) { ep_->closeAllFiles(); }
  void Machine::closeAllFiles(Stop const&) { ep_->closeAllFiles(); }

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
