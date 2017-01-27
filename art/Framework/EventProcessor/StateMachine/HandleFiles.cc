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

  void HandleFiles::openInputFile()
  {
    ep_.openInputFile();
    ep_.respondToOpenInputFile();
  }

  void HandleFiles::maybeTriggerOutputFileSwitch()
  {
    assert(stagingAllowed_);

    if (!ep_.outputsToClose()) return;

    // Don't trigger another switch if one is already in progress!
    if (switchInProgress_) return;

    post_event(Pause{});
    post_event(SwitchOutputFiles{});
    switchInProgress_ = true;
  }

  void HandleFiles::disallowStaging()
  {
    ep_.setOutputFileStatus(OutputFileStatus::StagedToSwitch);
    stagingAllowed_ = false;
  }

  void HandleFiles::openSomeOutputFiles()
  {
    if (!ep_.outputsToOpen()) return;

    ep_.openSomeOutputFiles();
    ep_.respondToOpenOutputFiles();
    switchInProgress_ = false;
    stagingAllowed_ = true;
  }

  NewInputFile::NewInputFile(my_context ctx) :
    my_base{ctx}
  {
    context<HandleFiles>().openInputFile();
  }

  sc::result NewInputFile::react(SwitchOutputFiles const&)
  {
    return discard_event();
  }

}
