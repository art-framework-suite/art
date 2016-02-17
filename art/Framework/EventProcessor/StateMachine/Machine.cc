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

  Starting::Starting(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.beginJob();
  }

  Starting::~Starting(){}

}
