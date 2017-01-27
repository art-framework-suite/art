#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "art/Framework/EventProcessor/EventProcessor.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;
using art::OutputFileStatus;

namespace statemachine {

  HandleFiles::HandleFiles(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {}

  NewInputFile::NewInputFile(my_context ctx)
    : my_base{ctx}
    , ep_{context<Machine>().ep()}
  {
    ep_.openInputFile();
  }

  sc::result NewInputFile::react(SwitchOutputFiles const&)
  {
    return discard_event();
  }

}
