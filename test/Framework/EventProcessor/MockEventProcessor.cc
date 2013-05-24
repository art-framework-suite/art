
/*

*/

#include "test/Framework/EventProcessor/MockEventProcessor.h"

#include <sstream>

namespace {
  // As each data item is read from the mock data it is
  // stored in one of these:
  struct token {
    int value;
    char id;
  };

  std::istream & operator>>(std::istream & is, token & t) {
    if(is >> t.id) is >> t.value;
    return is;
  }
}

namespace art {

  MockEventProcessor::MockEventProcessor(const std::string& mockData,
                                         std::ostream& output,
                                         const statemachine::FileMode& fileMode,
                                         bool handleEmptyRuns,
                                         bool handleEmptySubRuns) :
    mockData_(mockData),
    output_(output),
    fileMode_(fileMode),
    handleEmptyRuns_(handleEmptyRuns),
    handleEmptySubRuns_(handleEmptySubRuns),
    subRun_(SubRunID::firstSubRun()),
    shouldWeCloseOutput_(true),
    shouldWeEndLoop_(true),
    shouldWeStop_(false)  {
  }

  art::MockEventProcessor::StatusCode
  MockEventProcessor::runToCompletion() {
    statemachine::Machine myMachine(this,
                                    fileMode_,
                                    handleEmptyRuns_,
                                    handleEmptySubRuns_);


    myMachine.initiate();

    // Loop over the mock data items
    std::istringstream input(mockData_);
    token t;
    while (input >> t) {

      char ch = t.id;

      if (ch == 'r') {
        output_ << "    *** nextItemType: Run " << t.value << " ***\n";
        subRun_ = SubRunID::firstSubRun(RunID(t.value));
        myMachine.process_event( statemachine::Run(subRun_.runID()) );
      }
      else if (ch == 'l') {
        output_ << "    *** nextItemType: SubRun " << t.value << " ***\n";
        subRun_ = SubRunID(subRun_.run(), t.value);
        myMachine.process_event( statemachine::SubRun(subRun_) );
      }
      else if (ch == 'e') {
        output_ << "    *** nextItemType: Event ***\n";
        // a special value for test purposes only
        if (t.value == 7) {
          shouldWeStop_ = true;
          output_ << "    *** shouldWeStop will return true this event ***\n";
        }
        else {
          shouldWeStop_ = false;
        }
        myMachine.process_event( statemachine::Event() );
      }
      else if (ch == 'f') {
        output_ << "    *** nextItemType: File " << t.value << " ***\n";
        // a special value for test purposes only
        if (t.value == 0) shouldWeCloseOutput_ = false;
        else shouldWeCloseOutput_ = true;
        myMachine.process_event( statemachine::File() );
      }
      else if (ch == 's') {
        output_ << "    *** nextItemType: Stop " << t.value << " ***\n";
        // a special value for test purposes only
        if (t.value == 0) shouldWeEndLoop_ = false;
        else shouldWeEndLoop_ = true;
        myMachine.process_event( statemachine::Stop() );
      }
      else if (ch == 'x') {
        output_ << "    *** nextItemType: Restart " << t.value << " ***\n";
        shouldWeEndLoop_ = t.value;
        myMachine.process_event( statemachine::Restart() );
      }

      if (myMachine.terminated()) {
        output_ << "The state machine reports it has been terminated\n";
      }
    }
    return epSuccess;
  }

  // Not used, this one does nothing

  void MockEventProcessor::readFile() {
    output_ << " \treadFile\n";
  }

  void MockEventProcessor::closeInputFile() {
    output_ << "\tcloseInputFile\n";
  }

  void MockEventProcessor::openOutputFiles() {
    output_ << "\topenOutputFiles\n";
  }

  void MockEventProcessor::closeOutputFiles() {
    output_ << "\tcloseOutputFiles\n";
  }

  void MockEventProcessor::respondToOpenInputFile() {
    output_ << "\trespondToOpenInputFile\n";
  }

  void MockEventProcessor::respondToCloseInputFile() {
    output_ << "\trespondToCloseInputFile\n";
  }

  void MockEventProcessor::respondToOpenOutputFiles() {
    output_ << "\trespondToOpenOutputFiles\n";
  }

  void MockEventProcessor::respondToCloseOutputFiles() {
    output_ << "\trespondToCloseOutputFiles\n";
  }

  void MockEventProcessor::startingNewLoop() {
    output_ << "\tstartingNewLoop\n";
  }

  bool MockEventProcessor::endOfLoop() {
    output_ << "\tendOfLoop\n";
    return shouldWeEndLoop_;
  }

  void MockEventProcessor::rewindInput() {
    output_ << "\trewind\n";
  }

  void MockEventProcessor::prepareForNextLoop() {
    output_ << "\tprepareForNextLoop\n";
  }

  void MockEventProcessor::writeSubRunCache() {
    output_ << "\twriteSubRunCache\n";
  }

  void MockEventProcessor::writeRunCache() {
    output_ << "\twriteRunCache\n";
  }

  bool MockEventProcessor::shouldWeCloseOutput() const {
    output_ << "\tshouldWeCloseOutput\n";
    return shouldWeCloseOutput_;
  }

  void MockEventProcessor::doErrorStuff() {
    output_ << "\tdoErrorStuff\n";
  }

  void MockEventProcessor::beginRun(RunID run) {
    output_ << "\tbeginRun " << run.run() << "\n";
  }

  void MockEventProcessor::endRun(RunID run) {
    output_ << "\tendRun " << run.run() << "\n";
  }

  void MockEventProcessor::beginSubRun(SubRunID const & sr) {
    output_ << "\tbeginSubRun " << sr.run() << "/" << sr.subRun() << "\n";
  }

  void MockEventProcessor::endSubRun(SubRunID const & sr) {
    output_ << "\tendSubRun " << sr.run() << "/" << sr.subRun() << "\n";
  }

  RunID MockEventProcessor::readAndCacheRun() {
    output_ << "\treadAndCacheRun " << subRun_.run() << "\n";
    return subRun_.runID();
  }

  SubRunID MockEventProcessor::readAndCacheSubRun() {
    output_ << "\treadAndCacheSubRun " << subRun_.subRun() << "\n";
    return subRun_;
  }

  void MockEventProcessor::writeRun(RunID run) {
    output_ << "\twriteRun " << run.run() << "\n";
  }

  void MockEventProcessor::deleteRunFromCache(RunID run) {
    output_ << "\tdeleteRunFromCache " << run.run() << "\n";
  }

  void MockEventProcessor::writeSubRun(SubRunID const & sr) {
    output_ << "\twriteSubRun " << sr.run() << "/" << sr.subRun() << "\n";
  }

  void MockEventProcessor::deleteSubRunFromCache(SubRunID const & sr) {
    output_ << "\tdeleteSubRunFromCache " << sr.run() << "/" << sr.subRun() << "\n";
  }

  void MockEventProcessor::readEvent() {
    output_ << "\treadEvent\n";
  }

  void MockEventProcessor::processEvent() {
    output_ << "\tprocessEvent\n";
  }

  bool MockEventProcessor::shouldWeStop() const {
    output_ << "\tshouldWeStop\n";
    return shouldWeStop_;
  }

  void MockEventProcessor::setExceptionMessageFiles(std::string& /*message*/) { }
  void MockEventProcessor::setExceptionMessageRuns(std::string& /*message*/) { }
  void MockEventProcessor::setExceptionMessageSubRuns(std::string& /*message*/) { }

  bool MockEventProcessor::alreadyHandlingException() const { return false; }
}
