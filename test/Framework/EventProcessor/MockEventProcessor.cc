#include "art/Utilities/Exception.h"
#include "test/Framework/EventProcessor/MockEventProcessor.h"

#include <sstream>
#include <string>

namespace {

  std::uint32_t number(std::string const& action)
  {
    if (std::count(action.begin(), action.end(), ':') != 1u)
      throw art::Exception(art::errors::Configuration)
        << "The specified action for \"" << action << "\" must\n"
        << "contain only one ':'.";
    return std::stoul(action.substr(2)); // (e.g.) e:14 - start at "14"
  }

  std::string nextItemType(std::string const& event)
  {
    return "    *** nextItemType: "+event+" ***\n";
  }

  std::string internalPost(std::string const& event)
  {
    return "    *** internalPost w/in nextItemType: "+event+" ***\n";
  }

}

namespace art {

  MockEventProcessor::MockEventProcessor(std::string const& mockData,
                                         std::ostream& output,
                                         bool handleEmptyRuns,
                                         bool handleEmptySubRuns) :
    mockData_{mockData},
    output_{output},
    handleEmptyRuns_{handleEmptyRuns},
    handleEmptySubRuns_{handleEmptySubRuns}
  {}

  art::MockEventProcessor::StatusCode
  MockEventProcessor::runToCompletion() {

    statemachine::Machine machine{this, handleEmptyRuns_, handleEmptySubRuns_};
    machine.initiate();

    // Loop over the mock data items
    std::istringstream input {mockData_};
    for (std::string action; input >> action; ) {

      if (action == "InputFile") {
        output_ << nextItemType("InputFile");
        machine.process_event( statemachine::InputFile() );
      }
      else if (action == "InputFile+SwitchOutputFiles") {
        // Request output file switch along with InputFile switch.
        // This action is different than 'InputFile' followed by
        // 'SwitchOutputFiles' since the input file must be closed in
        // between output file closures and openings.
        closeAtBoundary_ = true;
        output_ << nextItemType("InputFile+SwitchOutputFiles");
        machine.process_event( statemachine::InputFile() );
      }
      else if (action == "SwitchOutputFiles") {
        // Do not process 'SwitchOutputFiles' event here!  That event
        // is posted internally by the state machine whenever a
        // request is made to close an output file.  The
        // 'SwitchOutputFiles' action simply makes the request,
        // achieved by setting 'closeAtBoundary_' to 'true'.
        closeAtBoundary_ = true;
        output_ << internalPost("SwitchOutputFiles");
      }
      else if (action == "Stop") {
        output_ << nextItemType("Stop");
        machine.process_event( statemachine::Stop() );
      }
      else if (action[0] == 'r') {
        readRun_ = RunID{number(action)};
        output_ << nextItemType("Run");
        machine.process_event( statemachine::Run(readRun_) );
      }
      else if (action[0] == 's') {
        readSubRun_ = SubRunID{run_, number(action)};
        output_ << nextItemType("SubRun");
        machine.process_event( statemachine::SubRun(readSubRun_) );
      }
      else if (action[0] == 'e') {
        readEvent_ = EventID{subRun_, number(action)};
        // a special value for test purposes only
        shouldWeStop_ = (readEvent_.event() == 7) ? true : false;
        output_ << nextItemType("Event");
        machine.process_event( statemachine::Event() );
      }
      else {
        throw art::Exception(art::errors::Configuration)
          << "Statemachine test pattern \"" << action << "\" not recognized.";
      }

      if (machine.terminated()) {
        output_ << "The state machine reports it has been terminated\n";
      }
    }
    return epSuccess;
  }

  // Not used, this one does nothing

  void MockEventProcessor::openInputFile() {
    output_ << " \topenInputFile\n";
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
    closeAtBoundary_ = false;
  }

  void MockEventProcessor::closeSomeOutputFiles(std::size_t const) {
    output_ << "\tcloseSomeOutputFiles\n";
  }

  void MockEventProcessor::switchOutputs(std::size_t const) {
    closeSomeOutputFiles({});
    openSomeOutputFiles({});
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
    output_ << "\tbeginRun..........(" << run << ")\n";
  }

  void MockEventProcessor::endRun(RunID run) {
    output_ << "\tendRun............(" << run << ")\n";
  }

  void MockEventProcessor::beginSubRun(SubRunID const & sr) {
    output_ << "\tbeginSubRun.......(" << sr <<")\n";
  }

  void MockEventProcessor::endSubRun(SubRunID const & sr) {
    output_ << "\tendSubRun.........(" << sr << ")\n";
  }

  RunID MockEventProcessor::runPrincipalID() const {
    return run_;
  }

  SubRunID MockEventProcessor::subRunPrincipalID() const {
    return subRun_;
  }

  EventID MockEventProcessor::eventPrincipalID() const {
    return event_;
  }

  bool MockEventProcessor::outputToCloseAtBoundary(Boundary const) const {
    return closeAtBoundary_;
  }

  RunID MockEventProcessor::readAndCacheRun() {
    run_ = readRun_;
    output_ << "\treadAndCacheRun...(" << run_ << ")\n";
    return run_;
  }

  SubRunID MockEventProcessor::readAndCacheSubRun() {
    subRun_ = readSubRun_;
    run_    = subRun_.runID();
    output_ << "\treadAndCacheSubRun(" << subRun_ << ")\n";
    return subRun_;
  }

  void MockEventProcessor::writeRun(RunID run) {
    output_ << "\twriteRun..........(" << run << ")\n";
  }

  void MockEventProcessor::writeSubRun(SubRunID const & sr) {
    output_ << "\twriteSubRun.......(" << sr << ")\n";
  }

  void MockEventProcessor::clearPrincipalCache() {
    output_ << "\tclearPrincipalCache\n";
  }

  void MockEventProcessor::readEvent() {
    event_  = readEvent_;
    subRun_ = event_.subRunID();
    run_    = subRun_.runID();
    output_ << "\treadEvent.........(" << event_ << ")\n";
  }

  void MockEventProcessor::processEvent() {
    output_ << "\tprocessEvent......(" << event_ << ")\n";
  }

  void MockEventProcessor::writeEvent() {
    output_ << "\twriteEvent........(" << event_ << ")\n";
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
