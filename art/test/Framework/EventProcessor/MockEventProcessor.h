#ifndef art_test_Framework_EventProcessor_MockEventProcessor_h
#define art_test_Framework_EventProcessor_MockEventProcessor_h

/*
  Version of the Event Processor used for tests of
  the state machine and other tests.
*/

#include "art/Framework/Core/IEventProcessor.h"
#include "art/Framework/EventProcessor/StateMachine/Machine.h"
#include <iostream>
#include <string>

namespace art
{
  class MockEventProcessor : public IEventProcessor {
  public:

    MockEventProcessor(std::string const& mockData,
                       std::ostream& output,
                       bool handleEmptyRuns,
                       bool handleEmptySubRuns);

    StatusCode runToCompletion() override;

    void openInputFile() override;
    void closeInputFile() override;
    void openAllOutputFiles() override;
    void closeAllOutputFiles() override;
    void openSomeOutputFiles() override;
    void closeSomeOutputFiles() override;
    void setOutputFileStatus(OutputFileStatus) override {}

    void respondToOpenInputFile() override;
    void respondToCloseInputFile() override;
    void respondToOpenOutputFiles() override;
    void respondToCloseOutputFiles() override;

    void rewindInput() override;
    void recordOutputClosureRequests(Boundary) override {}
    void incrementInputFileNumber() override {};
    bool outputsToOpen() const override;
    bool outputsToClose() const override;
    bool someOutputsOpen() const override;

    void doErrorStuff() override;

    void beginJob() override;
    void endJob() override;

    void beginRun() override;
    void endRun() override;

    void beginSubRun() override;
    void endSubRun() override;

    RunID readRun() override;
    SubRunID readSubRun() override;
    RunID runPrincipalID() const override;
    SubRunID subRunPrincipalID() const override;
    EventID eventPrincipalID() const override;
    void writeRun() override;
    void writeSubRun() override;
    void setRunAuxiliaryRangeSetID() override;
    void setSubRunAuxiliaryRangeSetID() override;
    void writeEvent() override;

    void readEvent() override;
    void processEvent() override;
    bool shouldWeStop() const override;

    void setExceptionMessageFiles(std::string const& message) override;
    void setExceptionMessageRuns(std::string const& message) override;
    void setExceptionMessageSubRuns(std::string const& message) override;

    bool alreadyHandlingException() const override;

    bool setTriggerPathEnabled(std::string const & name, bool enable) override;
    bool setEndPathModuleEnabled(std::string const & label, bool enable) override;

  private:
    std::string mockData_;
    std::ostream & output_;
    bool handleEmptyRuns_;
    bool handleEmptySubRuns_;

    RunID run_ {};
    RunID readRun_ {};
    SubRunID subRun_ {};
    SubRunID readSubRun_ {};
    EventID event_ {};
    EventID readEvent_ {};
    bool outputsToClose_ {false};
    bool outputsToOpen_ {false};
    bool shouldWeStop_ {false};
  };
}

#endif /* art_test_Framework_EventProcessor_MockEventProcessor_h */

// Local Variables:
// mode: c++
// End:
