#include "art/Framework/EventProcessor/EPStates.h"
#include "cetlib/exception.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

using art::Boundary;

namespace statemachine {
  Run::Run(art::RunID id) : id_{id} {}
  art::RunID Run::id() const { return id_; }

  SubRun::SubRun(art::SubRunID id) : id_{id} {}
  art::SubRunID const & SubRun::id() const { return id_; }

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

  //===================================================================

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

  //===================================================================

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
  bool HandleRuns::runException() const { return runException_; }

  void HandleRuns::setupCurrentRun()
  {
    runException_ = true;
    currentRun_ = ep_.readAndCacheRun();
    runException_ = false;
    if (context<Machine>().handleEmptyRuns()) {
      beginRun(currentRun());
    }
  }

  void HandleRuns::resumeAndFinalizeRun(Run const&)
  {
    resetFormerState();
    finalizeRun();
  }

  void HandleRuns::resume(SubRun const&)
  {
    resetFormerState();
  }

  void HandleRuns::resetFormerState()
  {
    finalizeEnabled_ = true;
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
    context<HandleFiles>().setCurrentBoundary(Boundary::Run);
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

  //===================================================================

  HandleSubRuns::HandleSubRuns(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()},
    currentSubRun_{ep_.subRunPrincipalID()}
  {
    // std::cout << " HandleSubRuns()\n";
    checkInvariant();
  }

  void HandleSubRuns::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    exitCalled_ = true;
    checkInvariant();
    finalizeAllSubRuns();
  }

  HandleSubRuns::~HandleSubRuns()
  {
    // std::cout << "~HandleSubRuns()\n";
    if (!exitCalled_) {
      try {
        checkInvariant();
        finalizeAllSubRuns();
      }
      catch (cet::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::bad_alloc const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns\n"
                << "after the primary exception.  We give up trying to clean up subRuns\n"
                << "at this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleSubRuns::finalizeAllSubRuns.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleSubRuns::finalizeAllSubRuns\n"
                << e.what() << "\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleSubRuns::finalizeAllSubRuns\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
    }
  }

  void HandleSubRuns::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
  }

  art::SubRunID const & HandleSubRuns::currentSubRun() const { return currentSubRun_; }

  void HandleSubRuns::resumeAndFinalizeSubRun(SubRun const&)
  {
    resetFormerState();
    finalizeSubRun();
  }

  void HandleSubRuns::resume(Event const&) { resetFormerState(); }

  void HandleSubRuns::resetFormerState()
  {
    finalizeEnabled_ = true;
  }

  bool HandleSubRuns::currentSubRunEmpty() const { return currentSubRunEmpty_; }

  std::vector<art::SubRunID> const & HandleSubRuns::unhandledSubRuns() const
  {
    return unhandledSubRuns_;
  }

  void HandleSubRuns::setupCurrentSubRun()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    subRunException_ = true;
    currentSubRun_ = ep_.readAndCacheSubRun();
    subRunException_ = false;
    currentSubRunEmpty_ = true;
  }

  void HandleSubRuns::finalizeAllSubRuns()
  {
    if (subRunException_ || context<HandleRuns>().runException()) return;
    if (finalizeEnabled_) {
      finalizeSubRun();
    }
    finalizeOutstandingSubRuns(); // always finalize unhandled subruns
  }

  void HandleSubRuns::finalizeSubRun(SubRun const&)
  {
    finalizeSubRun();
  }

  void HandleSubRuns::finalizeSubRun()
  {
    auto const& machine = context<Machine>();
    auto const& handleRuns = context<HandleRuns>();
    subRunException_ = true;

    if (currentSubRunEmpty_ && machine.handleEmptySubRuns() && !handleRuns.beginRunCalled() ) {
      unhandledSubRuns_.push_back(currentSubRun_);
    }
    else {
      if (currentSubRunEmpty_ && machine.handleEmptySubRuns() &&  handleRuns.beginRunCalled() ) {
        ep_.beginSubRun(currentSubRun_);
        ep_.endSubRun(currentSubRun_);
      }
      if (!currentSubRunEmpty_) {
        ep_.endSubRun(currentSubRun_);
      }
      ep_.writeSubRun(currentSubRun_);
      ep_.recordOutputClosureRequests();
      context<HandleFiles>().maybeTriggerOutputFileSwitch(Boundary::SubRun);
    }
    currentSubRun_ = art::SubRunID(); // Invalid.
    subRunException_ = false;
  }

  void HandleSubRuns::finalizeOutstandingSubRuns()
  {
    subRunException_ = true;
    for (auto const& sr : unhandledSubRuns_) {
      ep_.beginSubRun(sr);
      ep_.endSubRun(sr);
      ep_.writeSubRun(sr);
      ep_.recordOutputClosureRequests();
      context<HandleFiles>().maybeTriggerOutputFileSwitch(Boundary::SubRun);
    }
    unhandledSubRuns_.clear();
    subRunException_ = false;
  }

  void HandleSubRuns::markSubRunNonEmpty()
  {
    if (currentSubRunEmpty_) {
      finalizeOutstandingSubRuns();
      subRunException_ = true;
      ep_.beginSubRun(currentSubRun_);
      subRunException_ = false;
      currentSubRunEmpty_ = false;
    }
  }

  NewSubRun::NewSubRun(my_context ctx) :
    my_base{ctx}
  {
    // std::cout << " NewSubRun()\n";
    context<HandleFiles>().setCurrentBoundary(Boundary::SubRun);
    context<HandleSubRuns>().setupCurrentSubRun();
    checkInvariant();
  }

  NewSubRun::~NewSubRun()
  {
    // std::cout << "~NewSubRun()\n";
    checkInvariant();
  }

  void NewSubRun::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    assert(context<HandleSubRuns>().unhandledSubRuns().empty());
    assert(context<HandleSubRuns>().currentSubRunEmpty());
  }

  PauseSubRun::PauseSubRun(my_context ctx)
    : my_base{ctx}
  {
    // std::cout << " PauseSubRun()\n";
  }

  PauseSubRun::~PauseSubRun()
  {
    // std::cout << "~PauseSubRun()\n";
  }

  //===================================================================

  HandleEvents::HandleEvents(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()},
    currentEvent_{ep_.eventPrincipalID()}
  {
    // std::cout << " HandleEvents()\n";
  }

  void HandleEvents::checkInvariant()
  {
    assert(true);
  }

  void HandleEvents::exit()
  {
    if (ep_.alreadyHandlingException()) return;
    exitCalled_ = true;
    checkInvariant();
    finalizeEvent();
  }

  HandleEvents::~HandleEvents()
  {
    // std::cout << "~HandleEvents()\n";
    if (!exitCalled_) {
      try {
        checkInvariant();
        finalizeEvent();
      }
      catch (cet::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::bad_alloc const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleEvents::finalizeEvent.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (std::exception const& e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleEvents::finalizeEvent\n"
                << e.what() << "\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up an event after\n"
                << "the primary exception.  We give up trying to clean up an event at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleEvents::finalizeEvent\n";
        ep_.setExceptionMessageSubRuns(message.str());
      }
    }
  }

  void HandleEvents::finalizeEvent()
  {
    if (!finalizeEnabled_) return;
    ep_.writeEvent();
    ep_.recordOutputClosureRequests();
    context<HandleFiles>().maybeTriggerOutputFileSwitch(Boundary::Event);
  }

  void HandleEvents::resumeAndFinalizeEvent(Event const&)
  {
    finalizeEnabled_ = true;
    finalizeEvent();
  }

  NewEvent::NewEvent(my_context ctx) :
    my_base{ctx},
    ep_{context<Machine>().ep()}
  {
    // std::cout << " NewEvent()\n";
    context<HandleFiles>().setCurrentBoundary(Boundary::Event);
    readAndProcessEvent();
    checkInvariant();
  }

  NewEvent::~NewEvent()
  {
    // std::cout << "~NewEvent()\n";
    checkInvariant();
  }

  void NewEvent::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleRuns>().beginRunCalled());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    assert(context<HandleSubRuns>().unhandledSubRuns().empty());
    assert(!context<HandleSubRuns>().currentSubRunEmpty());
  }

  void NewEvent::readAndProcessEvent()
  {
    markNonEmpty();
    ep_.readEvent();
    ep_.processEvent();
    if (ep_.shouldWeStop()) { post_event(Stop()); }
  }

  void NewEvent::markNonEmpty()
  {
    context<HandleRuns>().beginRunIfNotDoneAlready();
    context<HandleSubRuns>().markSubRunNonEmpty();
  }

  PauseEvent::PauseEvent(my_context ctx)
    : my_base(ctx)
  {
    // std::cout << " PauseEvent()\n";
  }

  PauseEvent::~PauseEvent()
  {
    // std::cout << "~PauseEvent()\n";
  }

}
