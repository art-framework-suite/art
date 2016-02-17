#ifndef test_Framework_EventProcessor_MockEventProcessor_h
#define test_Framework_EventProcessor_MockEventProcessor_h

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
    void openSomeOutputFiles(std::size_t const) override;
    void closeSomeOutputFiles(std::size_t const) override;

    void respondToOpenInputFile() override;
    void respondToCloseInputFile() override;
    void respondToOpenOutputFiles() override;
    void respondToCloseOutputFiles() override;

    void rewindInput() override;
    void recordOutputClosureRequests() override {}
    void switchOutputs(std::size_t const) override;
    bool outputToCloseAtBoundary(Boundary const) const override;

    void doErrorStuff() override;

    void beginJob() override;
    void endJob() override;

    void beginRun(RunID run) override;
    void endRun(RunID run) override;

    void beginSubRun(SubRunID const & sr) override;
    void endSubRun(SubRunID const & sr) override;

    RunID readAndCacheRun() override;
    SubRunID readAndCacheSubRun() override;
    RunID runPrincipalID() const override;
    SubRunID subRunPrincipalID() const override;
    EventID eventPrincipalID() const override;
    void writeRun(RunID run) override;
    void writeSubRun(SubRunID const & sr) override;
    void clearPrincipalCache() override;
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
    bool closeAtBoundary_ {false};
    bool shouldWeStop_ {false};
  };
}

#endif /* test_Framework_EventProcessor_MockEventProcessor_h */

// Local Variables:
// mode: c++
// End:
