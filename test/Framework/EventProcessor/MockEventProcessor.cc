#include "art/Utilities/Exception.h"
#include "test/Framework/EventProcessor/MockEventProcessor.h"

#include <sstream>
#include <string>

namespace {

  class CloseAtBoundarySentry {
  public:

    CloseAtBoundarySentry(bool& close) : close_{close}
    {
      close_ = true;
    }

    ~CloseAtBoundarySentry()
    {
      close_ = false;
    }

  private:
    bool& close_;
  };

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
    std::string action;
    while (input >> action) {

      if (action == "InputFile") {
        output_ << nextItemType("InputFile");
        machine.process_event( statemachine::InputFile() );
      }
      else if (action == "InputFile+SwitchOutputFiles") {
        CloseAtBoundarySentry b {closeAtBoundary_};
        output_ << nextItemType("InputFile+SwitchOutputFiles");
        machine.process_event( statemachine::InputFile() );
      }
      else if (action == "SwitchOutputFiles") {
        CloseAtBoundarySentry b {closeAtBoundary_};
        output_ << nextItemType("SwitchOutputFiles");
        machine.process_event( statemachine::Pause() );
        machine.process_event( statemachine::SwitchOutputFiles() );
      }
      else if (action == "Stop") {
        output_ << nextItemType("Stop");
        machine.process_event( statemachine::Stop() );
      }
      else if (action[0] == 'r') {
        run_ = RunID{number(action)};
        output_ << nextItemType("Run");
        machine.process_event( statemachine::Run(run_) );
      }
      else if (action[0] == 's') {
        subRun_ = SubRunID{run_, number(action)};
        run_    = subRun_.runID();
        output_ << nextItemType("SubRun");
        machine.process_event( statemachine::SubRun(subRun_) );
      }
      else if (action[0] == 'e') {
        event_  = EventID{subRun_, number(action)};
        subRun_ = event_.subRunID();
        run_    = subRun_.runID();
        // a special value for test purposes only
        shouldWeStop_ = (event_.event() == 7) ? true : false;
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
  }

  void MockEventProcessor::closeSomeOutputFiles(std::size_t const) {
    output_ << "\tcloseSomeOutputFiles\n";
  }

  void MockEventProcessor::switchOutputs(std::size_t const) {
    output_ << "\tswitchOutputs\n";
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
    return subRun_.runID();
  }

  SubRunID MockEventProcessor::subRunPrincipalID() const {
    return subRun_;
  }

  EventID MockEventProcessor::eventPrincipalID() const {
    return EventID{};
  }

  RunID MockEventProcessor::readAndCacheRun() {
    output_ << "\treadAndCacheRun...(" << run_ << ")\n";
    return run_;
  }

  SubRunID MockEventProcessor::readAndCacheSubRun() {
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
