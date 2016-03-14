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

  void Machine::startingNewLoop(File const &)
  {
    ep_->startingNewLoop();
  }

  void Machine::startingNewLoop(Stop const &)
  {
    if (ep_->alreadyHandlingException()) { return; }
    ep_->startingNewLoop();
  }

  void Machine::rewindAndPrepareForNextLoop(Restart const &)
  {
    ep_->prepareForNextLoop();
    ep_->rewindInput();
  }

  Starting::Starting(my_context ctx) : my_base(ctx) { }

  Starting::~Starting() { }

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
      catch (cet::exception & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files after\n"
                << "the primary exception.  We give up trying to clean up files at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        std::string msg(message.str());
        ep_.setExceptionMessageFiles(msg);
      }
      catch (std::bad_alloc & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files\n"
                << "after the primary exception.  We give up trying to clean up files\n"
                << "at this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleFiles::closeFiles.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        std::string msg(message.str());
        ep_.setExceptionMessageFiles(msg);
      }
      catch (std::exception & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files after\n"
                << "the primary exception.  We give up trying to clean up files at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleFiles::closeFiles\n"
                << e.what() << "\n";
        std::string msg(message.str());
        ep_.setExceptionMessageFiles(msg);
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up files after\n"
                << "the primary exception.  We give up trying to clean up files at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleFiles::closeFiles\n";
        std::string msg(message.str());
        ep_.setExceptionMessageFiles(msg);
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
    ep_.writeSubRunCache();
    ep_.writeRunCache();
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

  bool HandleFiles::shouldWeCloseOutput()
  {
    if (context<Machine>().fileMode() == NOMERGE) { return true; }
    return ep_.shouldWeCloseOutput();
  }

  EndingLoop::EndingLoop(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    if (ep_.alreadyHandlingException() || ep_.endOfLoop()) { post_event(Stop()); }
    else { post_event(Restart()); }
  }

  EndingLoop::~EndingLoop() { }

  sc::result EndingLoop::react(Stop const &)
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

  Error::~Error() { }

  class HandleNewInputFile1;
  class NewInputAndOutputFiles;

  FirstFile::FirstFile(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  { }

  FirstFile::~FirstFile() { }

  sc::result FirstFile::react(File const &)
  {
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<HandleNewInputFile1>();
    }
  }

  HandleNewInputFile1::HandleNewInputFile1(my_context ctx) :
    my_base(ctx)
  {
    context<HandleFiles>().goToNewInputFile();
  }

  HandleNewInputFile1::~HandleNewInputFile1() { }

  sc::result HandleNewInputFile1::react(File const &)
  {
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<HandleNewInputFile1>();
    }
  }

  NewInputAndOutputFiles::NewInputAndOutputFiles(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    goToNewInputAndOutputFiles();
  }

  NewInputAndOutputFiles::~NewInputAndOutputFiles() { }

  sc::result NewInputAndOutputFiles::react(File const &)
  {
    if (context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<NewInputAndOutputFiles>();
    }
    else {
      return transit<HandleNewInputFile1>();
    }
  }

  void NewInputAndOutputFiles::goToNewInputAndOutputFiles()
  {
    ep_.writeSubRunCache();
    ep_.writeRunCache();
    ep_.respondToCloseOutputFiles();
    ep_.closeOutputFiles();
    ep_.respondToCloseInputFile();
    ep_.closeInputFile();
    ep_.readFile();
    ep_.respondToOpenInputFile();
    ep_.openOutputFiles();
    ep_.respondToOpenOutputFiles();
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
      catch (cet::exception & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs after\n"
                << "the primary exception.  We give up trying to clean up runs at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        std::string msg(message.str());
        ep_.setExceptionMessageRuns(msg);
      }
      catch (std::bad_alloc & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs\n"
                << "after the primary exception.  We give up trying to clean up runs\n"
                << "at this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleRuns::finalizeRun.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        std::string msg(message.str());
        ep_.setExceptionMessageRuns(msg);
      }
      catch (std::exception & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs after\n"
                << "the primary exception.  We give up trying to clean up runs at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleRuns::finalizeRun\n"
                << e.what() << "\n";
        std::string msg(message.str());
        ep_.setExceptionMessageRuns(msg);
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up runs after\n"
                << "the primary exception.  We give up trying to clean up runs at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleRuns::finalizeRun\n";
        std::string msg(message.str());
        ep_.setExceptionMessageRuns(msg);
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
    if (context<Machine>().fileMode() == MERGE) {
      if (previousRuns_.find(currentRun_) != previousRuns_.end()) {
        throw cet::exception("Merge failure:")
          << "Run " << currentRun_ << " is discontinuous, and cannot be merged in this mode.\n"
          "The run is split across two or more input files,\n"
          "and either the run is not the last run in the previous input file,\n"
          "or it is not the first run in the current input file.\n"
          "To handle this case, either sort the input files, if not sorted.\n";
      }
    }
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
    if (context<Machine>().fileMode() == NOMERGE ||
        context<Machine>().fileMode() == MERGE) {
      ep_.writeRun(currentRun_);
      ep_.deleteRunFromCache(currentRun_);
      previousRuns_.insert(currentRun_);
    }
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

  sc::result NewRun::react(File const &)
  {
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile2>();
    }
    return forward_event();
  }

  HandleNewInputFile2::HandleNewInputFile2(my_context ctx) :
    my_base(ctx)
  {
    context<HandleFiles>().goToNewInputFile();
    checkInvariant();
  }

  HandleNewInputFile2::~HandleNewInputFile2()
  {
    checkInvariant();
  }

  bool HandleNewInputFile2::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    return true;
  }

  sc::result HandleNewInputFile2::react(Run const & run)
  {
    checkInvariant();
    if (context<HandleRuns>().currentRun() != run.id()) {
      return transit<NewRun, HandleRuns, Run>(&HandleRuns::finalizeRun, run);
    }
    else {
      return transit<ContinueRun1>();
    }
  }

  sc::result HandleNewInputFile2::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile2>();
    }
    return forward_event();
  }

  ContinueRun1::ContinueRun1(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    ep_.readAndCacheRun();
    checkInvariant();
  }

  ContinueRun1::~ContinueRun1()
  {
    checkInvariant();
  }

  bool ContinueRun1::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    return true;
  }

  sc::result ContinueRun1::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile2>();
    }
    return forward_event();
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
      catch (cet::exception & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  The description of this additional exception follows:\n"
                << "cet::exception\n"
                << e.explain_self();
        std::string msg(message.str());
        ep_.setExceptionMessageSubRuns(msg);
      }
      catch (std::bad_alloc & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns\n"
                << "after the primary exception.  We give up trying to clean up subRuns\n"
                << "at this point.  This additional exception was a\n"
                << "std::bad_alloc exception thrown inside HandleSubRuns::finalizeAllSubRuns.\n"
                << "The job has probably exhausted the virtual memory available\n"
                << "to the process.\n";
        std::string msg(message.str());
        ep_.setExceptionMessageSubRuns(msg);
      }
      catch (std::exception & e) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  This additional exception was a\n"
                << "standard library exception thrown inside HandleSubRuns::finalizeAllSubRuns\n"
                << e.what() << "\n";
        std::string msg(message.str());
        ep_.setExceptionMessageSubRuns(msg);
      }
      catch (...) {
        std::ostringstream message;
        message << "------------------------------------------------------------\n"
                << "Another exception was caught while trying to clean up subRuns after\n"
                << "the primary exception.  We give up trying to clean up subRuns at\n"
                << "this point.  This additional exception was of unknown type and\n"
                << "thrown inside HandleSubRuns::finalizeAllSubRuns\n";
        std::string msg(message.str());
        ep_.setExceptionMessageSubRuns(msg);
      }
    }
  }

  bool HandleSubRuns::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    return true;
  }

  art::SubRunID const & HandleSubRuns::currentSubRun() const { return currentSubRun_; }

  bool HandleSubRuns::currentSubRunEmpty() const { return currentSubRunEmpty_; }

  std::vector<art::SubRunID> const & HandleSubRuns::unhandledSubRuns() const
  {
    return unhandledSubRuns_;
  }

  void HandleSubRuns::setupCurrentSubRun()
  {
    art::RunID run __attribute__((unused)) = context<HandleRuns>().currentRun();
    assert(run.isValid());
    subRunException_ = true;
    currentSubRun_ = ep_.readAndCacheSubRun();
    if (context<Machine>().fileMode() == MERGE) {
      if (previousSubRuns_.find(currentSubRun_) != previousSubRuns_.end()) {
        throw cet::exception("Merge failure:")
            << currentSubRun_
            << " is discontinuous, and cannot be merged in this mode.\n"
            "The subRun is split across two or more input files,\n"
            "and either the subRun is not the last run in the previous input file,\n"
            "or it is not the first subRun in the current input file.\n"
            "To handle this case, either sort the input files, if not sorted.\n";
      }
    }
    subRunException_ = false;
    currentSubRunEmpty_ = true;
  }

  void HandleSubRuns::finalizeAllSubRuns()
  {
    if (subRunException_ || context<HandleRuns>().runException()) { return; }
    finalizeSubRun();
    finalizeOutstandingSubRuns();
  }

  void HandleSubRuns::finalizeSubRun()
  {
    subRunException_ = true;
    if (currentSubRunEmpty_) {
      if (context<Machine>().handleEmptySubRuns()) {
        if (context<HandleRuns>().beginRunCalled()) {
          ep_.beginSubRun(currentSubRun_);
          ep_.endSubRun(currentSubRun_);
          if (context<Machine>().fileMode() == NOMERGE ||
              context<Machine>().fileMode() == MERGE) {
            ep_.writeSubRun(currentSubRun_);
            ep_.deleteSubRunFromCache(currentSubRun_);
            previousSubRuns_.insert(currentSubRun_);
          }
        }
        else {
          unhandledSubRuns_.push_back(currentSubRun_);
          previousSubRuns_.insert(currentSubRun_);
        }
      }
      else {
        if (context<Machine>().fileMode() == NOMERGE ||
            context<Machine>().fileMode() == MERGE) {
          ep_.writeSubRun(currentSubRun_);
          ep_.deleteSubRunFromCache(currentSubRun_);
          previousSubRuns_.insert(currentSubRun_);
        }
      }
    }
    else {
      ep_.endSubRun(currentSubRun_);
      if (context<Machine>().fileMode() == NOMERGE ||
          context<Machine>().fileMode() == MERGE) {
        ep_.writeSubRun(currentSubRun_);
        ep_.deleteSubRunFromCache(currentSubRun_);
        previousSubRuns_.insert(currentSubRun_);
      }
    }
    currentSubRun_ = art::SubRunID(); // Invalid.
    subRunException_ = false;
  }

  void HandleSubRuns::finalizeOutstandingSubRuns()
  {
    subRunException_ = true;
    for (auto iter = unhandledSubRuns_.begin();
         iter != unhandledSubRuns_.end();
         ++iter) {
      ep_.beginSubRun(*iter);
      ep_.endSubRun(*iter);
      if (context<Machine>().fileMode() == NOMERGE ||
          context<Machine>().fileMode() == MERGE) {
        ep_.writeSubRun(*iter);
        ep_.deleteSubRunFromCache(*iter);
        previousSubRuns_.insert(*iter);
      }
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

  FirstSubRun::FirstSubRun(my_context ctx) :
    my_base(ctx)
  {
    context<HandleSubRuns>().setupCurrentSubRun();
    checkInvariant();
  }

  FirstSubRun::~FirstSubRun()
  {
    checkInvariant();
  }

  bool FirstSubRun::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    assert(context<HandleSubRuns>().unhandledSubRuns().empty());
    assert(context<HandleSubRuns>().currentSubRunEmpty() == true);
    return true;
  }

  sc::result FirstSubRun::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile3>();
    }
    return forward_event();
  }

  AnotherSubRun::AnotherSubRun(my_context ctx) :
    my_base(ctx)
  {
    context<HandleSubRuns>().finalizeSubRun();
    context<HandleSubRuns>().setupCurrentSubRun();
    checkInvariant();
  }

  AnotherSubRun::~AnotherSubRun()
  {
    checkInvariant();
  }

  bool AnotherSubRun::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    assert(context<HandleSubRuns>().currentSubRunEmpty() == true);
    return true;
  }

  sc::result AnotherSubRun::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile3>();
    }
    return forward_event();
  }

  HandleEvent::HandleEvent(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    readAndProcessEvent();
    checkInvariant();
  }

  HandleEvent::~HandleEvent()
  {
    checkInvariant();
  }

  bool HandleEvent::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleRuns>().beginRunCalled());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    assert(context<HandleSubRuns>().unhandledSubRuns().empty());
    assert(context<HandleSubRuns>().currentSubRunEmpty() == false);
    return true;
  }

  sc::result HandleEvent::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile3>();
    }
    return forward_event();
  }

  void HandleEvent::readAndProcessEvent()
  {
    markNonEmpty();
    ep_.readEvent();
    ep_.processEvent();
    if (ep_.shouldWeStop()) { post_event(Stop()); }
  }

  void HandleEvent::markNonEmpty()
  {
    context<HandleRuns>().beginRunIfNotDoneAlready();
    context<HandleSubRuns>().markSubRunNonEmpty();
  }


  HandleNewInputFile3::HandleNewInputFile3(my_context ctx) :
    my_base(ctx)
  {
    context<HandleFiles>().goToNewInputFile();
    checkInvariant();
  }

  HandleNewInputFile3::~HandleNewInputFile3()
  {
    checkInvariant();
  }

  bool HandleNewInputFile3::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    return true;
  }

  sc::result HandleNewInputFile3::react(Run const & run)
  {
    checkInvariant();
    if (context<HandleRuns>().currentRun() == run.id()) {
      return transit<ContinueRun2>();
    }
    return forward_event();
  }

  sc::result HandleNewInputFile3::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile3>();
    }
    return forward_event();
  }

  ContinueRun2::ContinueRun2(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    ep_.readAndCacheRun();
    checkInvariant();
  }

  ContinueRun2::~ContinueRun2()
  {
    checkInvariant();
  }

  bool ContinueRun2::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    return true;
  }

  sc::result ContinueRun2::react(SubRun const & subRun)
  {
    checkInvariant();
    if (context<HandleSubRuns>().currentSubRun() != subRun.id()) {
      return transit<AnotherSubRun>();
    }
    else {
      return transit<ContinueSubRun>();
    }
  }

  sc::result ContinueRun2::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile3>();
    }
    return forward_event();
  }

  ContinueSubRun::ContinueSubRun(my_context ctx) :
    my_base(ctx),
    ep_(context<Machine>().ep())
  {
    ep_.readAndCacheSubRun();
    checkInvariant();
  }

  ContinueSubRun::~ContinueSubRun()
  {
    checkInvariant();
  }

  bool ContinueSubRun::checkInvariant()
  {
    assert(context<HandleRuns>().currentRun().isValid());
    assert(context<HandleSubRuns>().currentSubRun().runID() == context<HandleRuns>().currentRun());
    assert(context<HandleSubRuns>().currentSubRun().isValid());
    return true;
  }

  sc::result ContinueSubRun::react(File const &)
  {
    checkInvariant();
    if (!context<HandleFiles>().shouldWeCloseOutput()) {
      return transit<HandleNewInputFile3>();
    }
    return forward_event();
  }
}
