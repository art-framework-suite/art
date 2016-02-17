#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {

  HandleFiles::HandleFiles(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    // std::cout << " HandleFiles()\n";
    openAllFiles();
  }

  void HandleFiles::exit()
  {
    if (ep_.alreadyHandlingException()) { return; }
    exitCalled_ = true;
    closeAllFiles();
  }

  HandleFiles::~HandleFiles()
  {
    // std::cout << "~HandleFiles()\n";
    if (!exitCalled_) {
      try {
        closeAllFiles();
      }
      catch (cet::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files after\n"
                << "the primary exception.  We give up trying to clean up files at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        ep_.setExceptionMessageFiles(message.str());
      }
      catch (std::bad_alloc const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files\n"
                << "after the primary exception.  We give up trying to clean up files\n"
                << "at this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleFiles::closeFiles.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        ep_.setExceptionMessageFiles(message.str());
      }
      catch (std::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files after\n"
                << "the primary exception.  We give up trying to clean up files at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleFiles::closeFiles\n"
                << e.what() << "\n";
        ep_.setExceptionMessageFiles(message.str());
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files after\n"
                << "the primary exception.  We give up trying to clean up files at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleFiles::closeFiles\n";
        ep_.setExceptionMessageFiles(message.str());
      }
    }
  }

  void HandleFiles::openAllFiles()
  {
    ep_.openInputFile();
    ep_.respondToOpenInputFile();
    ep_.openAllOutputFiles();
    ep_.respondToOpenOutputFiles();
  }

  void HandleFiles::closeAllFiles()
  {
    ep_.respondToCloseOutputFiles();
    ep_.closeAllOutputFiles();
    ep_.respondToCloseInputFile();
    ep_.clearPrincipalCache();
    ep_.closeInputFile();
  }

  void HandleFiles::goToNewInputFile()
  {
    ep_.respondToCloseInputFile();
    ep_.clearPrincipalCache();
    ep_.closeInputFile();
    ep_.openInputFile();
    ep_.respondToOpenInputFile();
  }

  void HandleFiles::setCurrentBoundary(Boundary::BT const b)
  {
    previousBoundary_ = currentBoundary_;
    currentBoundary_ = b;
  }

  void HandleFiles::maybeTriggerOutputFileSwitch(Boundary::BT const b)
  {
    if (!ep_.outputToCloseAtBoundary(b)) return;

    // Don't trigger if a switch is already in progress!
    if (switchInProgress_) return;

    post_event(Pause());
    post_event(SwitchOutputFiles());
    switchInProgress_ = true;
  }

  void HandleFiles::maybeOpenOutputFiles()
  {
    if (!ep_.outputToCloseAtBoundary(Boundary::InputFile)) return;

    ep_.openSomeOutputFiles(Boundary::InputFile);
    ep_.respondToOpenOutputFiles();
    switchInProgress_ = false;
  }

  void HandleFiles::maybeCloseOutputFiles()
  {
    if (!ep_.outputToCloseAtBoundary(Boundary::InputFile)) return;

    ep_.respondToCloseOutputFiles();
    ep_.closeSomeOutputFiles(Boundary::InputFile);
  }

  void HandleFiles::switchOutputFiles(SwitchOutputFiles const&)
  {
    ep_.respondToCloseOutputFiles();
    // If the previous state was (e.g.) NewSubRun, and the current
    // state is NewEvent, and an output file needs to close, then it
    // should be allowed to.  Setting 'start' equal to
    // 'previousBoundary_' would result in the for-loop below not
    // being executed.  We therefore take the minimum of the
    // boundaries as the starting point, although the end point is
    // always the current boundary.
    auto const start = std::min(previousBoundary_, currentBoundary_);
    for(std::size_t b = start; b<=currentBoundary_; ++b)
      ep_.switchOutputs(b);
    ep_.respondToOpenOutputFiles();
    switchInProgress_ = false;
  }

  Stopping::Stopping(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    ep_.endJob();
    post_event(Stop());
  }

  sc::result Stopping::react(Stop const &)
  {
    return terminate();
  }

  Error::Error(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    post_event(Stop());
    ep_.doErrorStuff();
  }

  class NewInputFile;

  FirstFile::FirstFile(my_context ctx) :
    my_base{ctx}
  {}

  NewInputFile::NewInputFile(my_context ctx) :
    my_base{ctx}
  {
    auto& hf = context<HandleFiles>();
    hf.maybeCloseOutputFiles();
    hf.goToNewInputFile();
    hf.maybeOpenOutputFiles();
  }

  sc::result NewInputFile::react(SwitchOutputFiles const&)
  {
    return discard_event();
  }

}
