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
    shouldWeEndLoop_(true),
    shouldWeStop_(false)  {
  }

  art::MockEventProcessor::StatusCode
  MockEventProcessor::runToCompletion() {
    statemachine::Machine myMachine(this,
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
        myMachine.process_event( statemachine::InputFile() );
      }
      else if (ch == 's') {
        output_ << "    *** nextItemType: Stop " << t.value << " ***\n";
        // a special value for test purposes only
        if (t.value == 0) shouldWeEndLoop_ = false;
        else shouldWeEndLoop_ = true;
        myMachine.process_event( statemachine::Stop() );
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

  void MockEventProcessor::openAllOutputFiles() {
    output_ << "\topenAllOutputFiles\n";
  }

  void MockEventProcessor::closeAllOutputFiles() {
    output_ << "\tcloseAllOutputFiles\n";
  }

  void MockEventProcessor::openSomeOutputFiles(std::size_t const) {
    output_ << "\topenSomeOutputFiles\n";
  }

  void MockEventProcessor::closeSomeOutputFiles(std::size_t const) {
    output_ << "\tcloseSomeOutputFiles\n";
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

  void MockEventProcessor::rewindInput() {
    output_ << "\trewind\n";
  }

  void MockEventProcessor::doErrorStuff() {
    output_ << "\tdoErrorStuff\n";
  }

  void MockEventProcessor::beginJob() {
    output_ << "\tbeginJob\n";
  }

  void MockEventProcessor::endJob() {
    output_ << "\tendJob\n";
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

  RunID MockEventProcessor::runPrincipalID() const {
    return subRun_.runID();
  }

  SubRunID MockEventProcessor::subRunPrincipalID() const {
    return subRun_;
  }

  EventID MockEventProcessor::eventPrincipalID() const {
    return EventID{};
  }

  RunID MockEventProcessor::readAndCacheRun() {
    output_ << "\treadAndCacheRun " << subRun_.run() << "\n";
    return runPrincipalID();
  }

  SubRunID MockEventProcessor::readAndCacheSubRun() {
    output_ << "\treadAndCacheSubRun " << subRun_.subRun() << "\n";
    return subRunPrincipalID();
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

  void MockEventProcessor::clearPrincipalCache() {
    output_ << "\tclearPrincipalCache\n";
  }

  void MockEventProcessor::readEvent() {
    output_ << "\treadEvent\n";
  }

  void MockEventProcessor::processEvent() {
    output_ << "\tprocessEvent\n";
  }

  void MockEventProcessor::writeEvent() {
    output_ << "\twriteEvent\n";
  }

  bool MockEventProcessor::shouldWeStop() const {
    output_ << "\tshouldWeStop\n";
    return shouldWeStop_;
  }

  void MockEventProcessor::setExceptionMessageFiles(std::string const& /*message*/) { }
  void MockEventProcessor::setExceptionMessageRuns(std::string const& /*message*/) { }
  void MockEventProcessor::setExceptionMessageSubRuns(std::string const& /*message*/) { }

  bool MockEventProcessor::alreadyHandlingException() const { return false; }

  bool MockEventProcessor::setTriggerPathEnabled(std::string const &, bool) { return true; }
  bool MockEventProcessor::setEndPathModuleEnabled(std::string const &, bool) { return true; }

}
