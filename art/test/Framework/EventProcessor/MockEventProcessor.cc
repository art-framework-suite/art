#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Utilities/Exception.h"
#include "art/test/Framework/EventProcessor/MockEventProcessor.h"

#include <cctype>
#include <sstream>
#include <string>

using namespace art;

namespace {

  std::uint32_t number(std::string const& action)
  {
    if (std::count(action.begin(), action.end(), ':') != 1u)
      throw Exception{errors::Configuration}
        << "The specified action for \"" << action << "\" must\n"
        << "contain only one ':'.";
    auto const level = action.substr(0,1);
    auto const symbol = action.substr(2);
    if (symbol.empty())
      throw Exception{errors::Configuration}
        << "The symbol for \"" << action << "\" is empty.\n"
        << "Please provide a positive number, or the character 'f'.\n";

    // For flush values -- r:f, s:f, or e:f
    if (std::isalpha(symbol[0])) {
      if (symbol[0] != 'f') {
        throw Exception{errors::Configuration}
          << "The character specified for a symbol must be 'f'.\n";
      }
      switch (level[0]) {
      case 'r': return IDNumber<Level::Run>::flush_value();
      case 's': return IDNumber<Level::SubRun>::flush_value();
      case 'e': return IDNumber<Level::Event>::flush_value();
      default:
        throw Exception{errors::Configuration}
          << "Action specifying flush value does not correspond to 'r', 's', or 'e'.\n";
      }
    }

    // For everything else
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
    bool firstAction {true};
    for (std::string action; input >> action;) {

      if (action == "InputFile") {
        output_ << nextItemType("InputFile");
        machine.process_event(statemachine::InputFile());
      }
      else if (action == "InputFile+OutputFiles") {
        if (!firstAction)
          throw Exception{errors::Configuration}
            << "Statemachine test pattern \"" << action << "\" can only appear as\n"
            << "first action in the requested action list.\n";
        // Request output file switch along with InputFile switch.
        // This action is different than 'InputFile' followed by
        // 'SwitchOutputFiles' since the input file must be closed in
        // between output file closures and openings.
        outputsToOpen_ = true;
        output_ << nextItemType("InputFile+OutputFiles");
        machine.process_event(statemachine::InputFile());
      }
      else if (action == "InputFile+SwitchOutputFiles") {
        // Request output file switch along with InputFile switch.
        // This action is different than 'InputFile' followed by
        // 'SwitchOutputFiles' since the input file must be closed in
        // between output file closures and openings.
        outputsToClose_ = true;
        output_ << nextItemType("InputFile+SwitchOutputFiles");
        machine.process_event(statemachine::InputFile());
      }
      else if (action == "SwitchOutputFiles") {
        // Do not process 'SwitchOutputFiles' event here!  That event
        // is posted internally by the state machine whenever a
        // request is made to close an output file.  The
        // 'SwitchOutputFiles' action simply makes the request,
        // achieved by setting 'closeAtBoundary_' to 'true'.
        outputsToClose_ = true;
        output_ << internalPost("SwitchOutputFiles");
      }
      else if (action == "Stop") {
        output_ << nextItemType("Stop");
        machine.process_event(statemachine::Stop());
      }
      else if (action[0] == 'r') {
        auto const r = number(action);
        readRun_ = (r == IDNumber<Level::Run>::flush_value()) ? RunID::flushRun() : RunID{r};
        output_ << nextItemType("Run");
        machine.process_event(statemachine::Run(readRun_));
      }
      else if (action[0] == 's') {
        auto const sr = number(action);
        readSubRun_ = (sr == IDNumber<Level::SubRun>::flush_value()) ? SubRunID::flushSubRun() : SubRunID{run_, sr};
        output_ << nextItemType("SubRun");
        machine.process_event(statemachine::SubRun(readSubRun_));
      }
      else if (action[0] == 'e') {
        auto const e = number(action);
        readEvent_ = (e == IDNumber<Level::Event>::flush_value()) ? EventID::flushEvent() : EventID{subRun_, e};
        // a special value for test purposes only
        shouldWeStop_ = (readEvent_.event() == 7);
        output_ << nextItemType("Event");
        machine.process_event(statemachine::Event());
      }
      else {
        throw Exception{errors::Configuration}
          << "Statemachine test pattern \"" << action << "\" not recognized.";
      }

      if (machine.terminated()) {
        output_ << "The state machine reports it has been terminated\n";
      }
      firstAction = false;
    }
    return epSuccess;
  }

  // Not used, this one does nothing

  void MockEventProcessor::openInputFile()
  {
    output_ << " \topenInputFile\n";
  }

  void MockEventProcessor::closeInputFile()
  {
    output_ << "\tcloseInputFile\n";
  }

  void MockEventProcessor::openAllOutputFiles()
  {
    output_ << "\topenAllOutputFiles\n";
  }

  void MockEventProcessor::closeAllOutputFiles()
  {
    output_ << "\tcloseAllOutputFiles\n";
  }

  void MockEventProcessor::openSomeOutputFiles()
  {
    output_ << "\topenSomeOutputFiles\n";
    outputsToOpen_ = false;
  }

  void MockEventProcessor::closeSomeOutputFiles()
  {
    output_ << "\tcloseSomeOutputFiles\n";
    outputsToClose_ = false;
    outputsToOpen_ = true;
  }

  void MockEventProcessor::respondToOpenInputFile()
  {
    output_ << "\trespondToOpenInputFile\n";
  }

  void MockEventProcessor::respondToCloseInputFile()
  {
    output_ << "\trespondToCloseInputFile\n";
  }

  void MockEventProcessor::respondToOpenOutputFiles()
  {
    output_ << "\trespondToOpenOutputFiles\n";
  }

  void MockEventProcessor::respondToCloseOutputFiles()
  {
    output_ << "\trespondToCloseOutputFiles\n";
  }

  void MockEventProcessor::rewindInput()
  {
    output_ << "\trewind\n";
  }

  void MockEventProcessor::doErrorStuff()
  {
    output_ << "\tdoErrorStuff\n";
  }

  void MockEventProcessor::beginJob() {
    output_ << "\tbeginJob\n";
  }

  void MockEventProcessor::endJob()
  {
    output_ << "\tendJob\n";
  }

  void MockEventProcessor::beginRun()
  {
    output_ << "\tbeginRun....................(" << run_ << ")\n";
  }

  void MockEventProcessor::endRun()
  {
    output_ << "\tendRun......................(" << run_ << ")\n";
  }

  void MockEventProcessor::beginSubRun()
  {
    output_ << "\tbeginSubRun.................(" << subRun_ <<")\n";
  }

  void MockEventProcessor::endSubRun()
  {
    output_ << "\tendSubRun...................(" << subRun_ << ")\n";
  }

  RunID MockEventProcessor::runPrincipalID() const
  {
    return run_;
  }

  SubRunID MockEventProcessor::subRunPrincipalID() const
  {
    return subRun_;
  }

  EventID MockEventProcessor::eventPrincipalID() const
  {
    return event_;
  }

  bool MockEventProcessor::outputsToClose() const
  {
    return outputsToClose_;
  }

  bool MockEventProcessor::outputsToOpen() const
  {
    return outputsToOpen_;
  }

  bool MockEventProcessor::someOutputsOpen() const
  {
    return true;
  }

  RunID MockEventProcessor::readRun()
  {
    run_ = readRun_;
    output_ << "\treadRun.....................(" << run_ << ")\n";
    return run_;
  }

  SubRunID MockEventProcessor::readSubRun()
  {
    subRun_ = readSubRun_;
    run_    = subRun_.runID();
    output_ << "\treadSubRun..................(" << subRun_ << ")\n";
    return subRun_;
  }

  void MockEventProcessor::writeRun()
  {
    output_ << "\twriteRun....................(" << run_ << ")\n";
  }

  void MockEventProcessor::writeSubRun()
  {
    output_ << "\twriteSubRun.................(" << subRun_ << ")\n";
  }

  void MockEventProcessor::setRunAuxiliaryRangeSetID()
  {
    output_ << "\tsetRunAuxiliaryRangeSetID...(" << run_ << ")\n";
  }

  void MockEventProcessor::setSubRunAuxiliaryRangeSetID()
  {
    output_ << "\tsetSubRunAuxiliaryRangeSetID(" << subRun_ << ")\n";
  }

  void MockEventProcessor::readEvent()
  {
    event_  = readEvent_;
    subRun_ = event_.subRunID();
    run_    = subRun_.runID();
    output_ << "\treadEvent....................(" << event_ << ")\n";
  }

  void MockEventProcessor::processEvent()
  {
    output_ << "\tprocessEvent.................(" << event_ << ")\n";
  }

  void MockEventProcessor::writeEvent()
  {
    output_ << "\twriteEvent...................(" << event_ << ")\n";
  }

  bool MockEventProcessor::shouldWeStop() const
  {
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
