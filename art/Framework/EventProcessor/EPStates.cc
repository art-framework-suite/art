#include "art/Framework/EventProcessor/EPStates.h"
#include "art/Framework/Core/IEventProcessor.h"
#include "cetlib/exception.h"

#include <exception>
#include <sstream>
#include <string>

namespace statemachine {
  Run::Run(art::RunID id) : id_(id) {}
  art::RunID Run::id() const { return id_; }

  SubRun::SubRun(art::SubRunID id) : id_(id) {}
  art::SubRunID const & SubRun::id() const { return id_; }

  Machine::Machine(art::IEventProcessor * ep,
                   FileMode fileMode,
                   bool handleEmptyRuns,
                   bool handleEmptySubRuns) :
    ep_(ep),
    fileMode_(fileMode),
    handleEmptyRuns_(handleEmptyRuns),
    handleEmptySubRuns_(handleEmptySubRuns) { }

  art::IEventProcessor & Machine::ep() const { return *ep_; }
  FileMode Machine::fileMode() const { return fileMode_; }
  bool Machine::handleEmptyRuns() const { return handleEmptyRuns_; }
  bool Machine::handleEmptySubRuns() const { return handleEmptySubRuns_; }

  Starting::Starting(my_context ctx) : my_base(ctx) { }

  Starting::~Starting(){}

  HandleFiles::HandleFiles(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep()),
    exitCalled_(false)
  {
    openFiles();
  }

  void HandleFiles::exit()
  {
    if (ep_.alreadyHandlingException()) { return; }
    exitCalled_ = true;
    closeFiles();
  }

  HandleFiles::~HandleFiles()
  {
    if (!exitCalled_) {
      try {
        closeFiles();
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

  void HandleFiles::openFiles()
  {
    ep_.readFile();
    ep_.respondToOpenInputFile();
    ep_.openOutputFiles();
    ep_.respondToOpenOutputFiles();
  }

  void HandleFiles::closeFiles()
  {
    ep_.respondToCloseOutputFiles();
    ep_.closeOutputFiles();
    ep_.respondToCloseInputFile();
    ep_.closeInputFile();
  }

  void HandleFiles::goToNewInputFile()
  {
    ep_.respondToCloseInputFile();
    ep_.closeInputFile();
    ep_.readFile();
    ep_.respondToOpenInputFile();
  }

  void HandleFiles::goToNewInputAndOutputFiles()
  {
    ep_.respondToCloseOutputFiles();
    ep_.closeOutputFiles();
    ep_.respondToCloseInputFile();
    ep_.closeInputFile();
    ep_.readFile();
    ep_.respondToOpenInputFile();
    ep_.openOutputFiles();
    ep_.respondToOpenOutputFiles();
  }

  void HandleFiles::goToNewOutputFiles()
  {
    ep_.respondToCloseOutputFiles();
    ep_.closeOutputFiles();
    ep_.openOutputFiles();
    ep_.respondToOpenOutputFiles();
  }

  bool HandleFiles::shouldWeCloseOutput()
  {
    if (context<Machine>().fileMode() == NOMERGE) { return true; }
    return ep_.shouldWeCloseOutput();
  }

  Stopping::Stopping(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    post_event(Stop());
  }

  sc::result Stopping::react(Stop const &)
  {
    return terminate();
  }

  Error::Error(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    post_event(Stop());
    ep_.doErrorStuff();
  }

  class NewInputFile;
  class NewInputAndOutputFiles;

  FirstFile::FirstFile(my_context ctx) :
    my_base(ctx)
  { }

  sc::result FirstFile::react(InputFile const &)
  {
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<NewInputFile>();
    }
  }

  NewInputFile::NewInputFile(my_context ctx) :
    my_base(ctx)
  {
    context<HandleFiles>().goToNewInputFile();
  }

  sc::result NewInputFile::react(InputFile const &)
  {
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<NewInputFile>();
    }
  }

  NewInputAndOutputFiles::NewInputAndOutputFiles(my_context ctx) :
    my_base(ctx)
  {
    context<HandleFiles>().goToNewInputAndOutputFiles();
  }

  sc::result NewInputAndOutputFiles::react(InputFile const &)
  {
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<NewInputFile>();
    }
  }

  HandleRuns::HandleRuns(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep()),
    exitCalled_(false),
    beginRunCalled_(false),
    currentRun_(),
    runException_(false) { }

  void HandleRuns::exit()
  {
    if (ep_.alreadyHandlingException()) { return; }
    exitCalled_ = true;
    finalizeRun();
  }

  HandleRuns::~HandleRuns()
  {
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
    if (runException_) { return; }
    runException_ = true;
    if (beginRunCalled_) { endRun(currentRun()); }
    ep_.writeRun(currentRun_);
    ep_.deleteRunFromCache(currentRun_);
    previousRuns_.insert(currentRun_);
    currentRun_ = art::RunID(); // Invalid.
    runException_ = false;
  }

  void HandleRuns::beginRunIfNotDoneAlready()
  {
    if (!beginRunCalled_) { beginRun(currentRun()); }
  }

  NewRun::NewRun(my_context ctx) :
    my_base(ctx)
  {
    assert(!context<HandleRuns>().currentRun().isValid());
    context<HandleRuns>().setupCurrentRun();
    // Here we assume that the input source or event processor
    // will throw if we fail to get a valid run.  Therefore
    // we should not ever fail this assert.
    assert(context<HandleRuns>().currentRun().isValid());
  }

  NewRun::~NewRun() { }

  sc::result NewRun::react(InputFile const &)
  {
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<NewInputFile>();
    }
  }

  HandleSubRuns::HandleSubRuns(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep()),
    exitCalled_(false),
    currentSubRunEmpty_(true),
    currentSubRun_(),
    previousSubRuns_(),
    subRunException_(false)
  {
    checkInvariant();
  }

  void HandleSubRuns::exit()
  {
    if (ep_.alreadyHandlingException()) { return; }
    exitCalled_ = true;
    checkInvariant();
    finalizeAllSubRuns();
  }

  HandleSubRuns::~HandleSubRuns()
  {
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
    if (subRunException_ || context<HandleRuns>().runException()) { return; }
    finalizeSubRun();
    finalizeOutstandingSubRuns();
  }

  void HandleSubRuns::finalizeSubRun(SubRun const&)
  {
    finalizeSubRun();
  }

  void HandleSubRuns::finalizeSubRun()
  {
    subRunException_ = true;
    if (currentSubRunEmpty_) {
      if (context<Machine>().handleEmptySubRuns()) {
        if (context<HandleRuns>().beginRunCalled()) {
          ep_.beginSubRun(currentSubRun_);
          ep_.endSubRun(currentSubRun_);
          ep_.writeSubRun(currentSubRun_);
          ep_.deleteSubRunFromCache(currentSubRun_);
        }
        else {
          unhandledSubRuns_.push_back(currentSubRun_);
        }
      }
      else {
        ep_.writeSubRun(currentSubRun_);
        ep_.deleteSubRunFromCache(currentSubRun_);
      }
    }
    else {
      ep_.endSubRun(currentSubRun_);
      ep_.writeSubRun(currentSubRun_);
      ep_.deleteSubRunFromCache(currentSubRun_);
    }
    previousSubRuns_.insert(currentSubRun_);
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
      ep_.deleteSubRunFromCache(sr);
      previousSubRuns_.insert(sr);
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
    my_base(ctx)
  {
    context<HandleSubRuns>().setupCurrentSubRun();
    checkInvariant();
  }

  NewSubRun::~NewSubRun()
  {
    checkInvariant();
  }

  void NewSubRun::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    assert(context<HandleSubRuns>().unhandledSubRuns().empty());
    assert(context<HandleSubRuns>().currentSubRunEmpty() == true);
  }

  sc::result NewSubRun::react(InputFile const &)
  {
    checkInvariant();
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<NewInputFile>();
    }
  }

  NewEvent::NewEvent(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    readAndProcessEvent();
    checkInvariant();
  }

  NewEvent::~NewEvent()
  {
    checkInvariant();
  }

  void NewEvent::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleRuns>().beginRunCalled());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    assert(context<HandleSubRuns>().unhandledSubRuns().empty());
    assert(context<HandleSubRuns>().currentSubRunEmpty() == false);
  }

  sc::result NewEvent::react(InputFile const &)
  {
    checkInvariant();
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<NewInputFile>();
    }
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

}
