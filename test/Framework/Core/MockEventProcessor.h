#ifndef test_Framework_Core_MockEventProcessor_h
#define test_Framework_Core_MockEventProcessor_h

/*


Version of the Event Processor used for tests of
the state machine and other tests.

Original Authors: W. David Dagenhart, Marc Paterno
*/

#include "art/Framework/Core/IEventProcessor.h"
#include "art/Framework/Core/EPStates.h"
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

    virtual StatusCode runToCompletion(bool onlineStateTransitions);
    virtual StatusCode runEventCount(int numberOfEventsToProcess);

    virtual void readFile();
    virtual void closeInputFile();
    virtual void openOutputFiles();
    virtual void closeOutputFiles();

    virtual void respondToOpenInputFile();
    virtual void respondToCloseInputFile();
    virtual void respondToOpenOutputFiles();
    virtual void respondToCloseOutputFiles();

    virtual void startingNewLoop();
    virtual bool endOfLoop();
    virtual void rewindInput();
    virtual void prepareForNextLoop();
    virtual void writeSubRunCache();
    virtual void writeRunCache();
    virtual bool shouldWeCloseOutput() const;

    virtual void doErrorStuff();

    virtual void beginRun(int run);
    virtual void endRun(int run);

    virtual void beginSubRun(int run, int subRun);
    virtual void endSubRun(int run, int subRun);

    virtual int readAndCacheRun();
    virtual int readAndCacheSubRun();
    virtual void writeRun(int run);
    virtual void deleteRunFromCache(int run);
    virtual void writeSubRun(int run, int subRun);
    virtual void deleteSubRunFromCache(int run, int subRun);

    virtual void readEvent();
    virtual void processEvent();
    virtual bool shouldWeStop() const;

    virtual void setExceptionMessageFiles(std::string& message);
    virtual void setExceptionMessageRuns(std::string& message);
    virtual void setExceptionMessageSubRuns(std::string& message);

    virtual bool alreadyHandlingException() const;

  private:
    std::string mockData_;
    std::ostream & output_;
    statemachine::FileMode fileMode_;
    bool handleEmptyRuns_;
    bool handleEmptySubRuns_;

    int run_;
    int subRun_;

    bool shouldWeCloseOutput_;
    bool shouldWeEndLoop_;
    bool shouldWeStop_;
  };
}

#endif /* test_Framework_Core_MockEventProcessor_h */

// Local Variables:
// mode: c++
// End:
