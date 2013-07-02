#ifndef test_Framework_EventProcessor_MockEventProcessor_h
#define test_Framework_EventProcessor_MockEventProcessor_h

/*


Version of the Event Processor used for tests of
the state machine and other tests.

Original Authors: W. David Dagenhart, Marc Paterno
*/

#include "art/Framework/Core/IEventProcessor.h"
#include "art/Framework/EventProcessor/EPStates.h"
#include <iostream>
#include <string>

namespace art
{
  class MockEventProcessor : public IEventProcessor {
  public:

    MockEventProcessor(const std::string& mockData,
                       std::ostream& output,
                       const statemachine::FileMode& fileMode,
                       bool handleEmptyRuns,
                       bool handleEmptySubRuns);

    StatusCode runToCompletion() override;

    void readFile() override;
    void closeInputFile() override;
    void openOutputFiles() override;
    void closeOutputFiles() override;

    void respondToOpenInputFile() override;
    void respondToCloseInputFile() override;
    void respondToOpenOutputFiles() override;
    void respondToCloseOutputFiles() override;

    void startingNewLoop() override;
    bool endOfLoop() override;
    void rewindInput() override;
    void prepareForNextLoop() override;
    void writeSubRunCache() override;
    void writeRunCache() override;
    bool shouldWeCloseOutput() const override;

    void doErrorStuff() override;

    void beginRun(RunID run) override;
    void endRun(RunID run) override;

    void beginSubRun(SubRunID const & sr) override;
    void endSubRun(SubRunID const & sr) override;

    RunID readAndCacheRun() override;
    SubRunID readAndCacheSubRun() override;
    void writeRun(RunID run) override;
    void deleteRunFromCache(RunID run) override;
    void writeSubRun(SubRunID const & sr) override;
    void deleteSubRunFromCache(SubRunID const & sr) override;

    void readEvent() override;
    void processEvent() override;
    bool shouldWeStop() const override;

    void setExceptionMessageFiles(std::string& message) override;
    void setExceptionMessageRuns(std::string& message) override;
    void setExceptionMessageSubRuns(std::string& message) override;

    bool alreadyHandlingException() const override;

  private:
    std::string mockData_;
    std::ostream & output_;
    statemachine::FileMode fileMode_;
    bool handleEmptyRuns_;
    bool handleEmptySubRuns_;

    SubRunID subRun_;

    bool shouldWeCloseOutput_;
    bool shouldWeEndLoop_;
    bool shouldWeStop_;
  };
}

#endif /* test_Framework_EventProcessor_MockEventProcessor_h */

// Local Variables:
// mode: c++
// End:
