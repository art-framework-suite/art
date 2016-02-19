#ifndef art_Framework_EventProcessor_EPStates_h
#define art_Framework_EventProcessor_EPStates_h

// ======================================================================
//
// The state machine that controls the processing of runs, subRun blocks,
// events, and loops is implemented using the boost statechart library
// and the states and events defined here.  This machine is used by the
// EventProcessor.
//
// ======================================================================

#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "boost/statechart/event.hpp"
#include "boost/statechart/state_machine.hpp"
#include "boost/mpl/list.hpp"
#include "boost/statechart/custom_reaction.hpp"
#include "boost/statechart/state.hpp"
#include "boost/statechart/transition.hpp"

#include <set>
#include <utility>
#include <vector>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

namespace art {
  class IEventProcessor;
}

namespace statemachine {

  enum FileMode { NOMERGE, MERGE, FULLLUMIMERGE, FULLMERGE };

  // Define the classes representing the "boost statechart events".
  // There are six of them.

  class Run : public sc::event<Run> {
  public:
    Run(art::RunID id);
    art::RunID id() const;
  private:
    art::RunID id_;
  };

  class SubRun : public sc::event<SubRun> {
  public:
    SubRun(art::SubRunID id);
    art::SubRunID const & id() const;
  private:
    art::SubRunID id_;
  };

  // It is slightly confusing that this one refers to
  // both physics event and a boost statechart event ...
  class Event : public sc::event<Event> { };

  class File : public sc::event<File> {};
  class Stop : public sc::event<Stop> {};
  class Restart : public sc::event<Restart> {};

  // Now define the machine and the states.
  // For all these classes, the first template argument
  // to the base class is the derived class.  The second
  // argument is the parent state or if it is a top level
  // state the Machine.  If there is a third template
  // argument it is the substate that is entered
  // by default on entry.

  class Starting;

  class Machine : public sc::state_machine<Machine, Starting>
  {
  public:
    Machine(art::IEventProcessor* ep,
            FileMode fileMode,
            bool handleEmptyRuns,
            bool handleEmptySubRuns);

    art::IEventProcessor& ep() const;
    FileMode fileMode() const;
    bool handleEmptyRuns() const;
    bool handleEmptySubRuns() const;

    void startingNewLoop(File const& file);
    void startingNewLoop(Stop const& stop);
    void rewindAndPrepareForNextLoop(Restart const& restart);

  private:

    art::IEventProcessor* ep_;
    FileMode fileMode_;
    bool handleEmptyRuns_;
    bool handleEmptySubRuns_;
  };

  class Error;
  class HandleFiles;
  class EndingLoop;

  class Starting : public sc::state<Starting, Machine>
  {
  public:
    Starting(my_context ctx);
    ~Starting();

    typedef mpl::list<
      sc::transition<Event, Error>,
      sc::transition<SubRun, Error>,
      sc::transition<Run, Error>,
      sc::transition<File, HandleFiles, Machine, &Machine::startingNewLoop>,
      sc::transition<Stop, EndingLoop, Machine, &Machine::startingNewLoop>,
      sc::transition<Restart, Error> > reactions;
  };

  class FirstFile;

  class HandleFiles : public sc::state<HandleFiles, Machine, FirstFile>
  {
  public:
    HandleFiles(my_context ctx);
    void exit();
    ~HandleFiles();

    typedef mpl::list<
      sc::transition<Event, Error>,
      sc::transition<SubRun, Error>,
      sc::transition<Run, Error>,
      sc::transition<File, Error>,
      sc::transition<Stop, EndingLoop>,
      sc::transition<Restart, Error> > reactions;

    void openFiles();
    void closeFiles();
    void goToNewInputFile();
    bool shouldWeCloseOutput();
  private:
    art::IEventProcessor & ep_;
    bool exitCalled_;
  };

  class EndingLoop : public sc::state<EndingLoop, Machine>
  {
  public:
    EndingLoop(my_context ctx);
    ~EndingLoop();
    typedef mpl::list<
      sc::transition<Restart, Starting, Machine, &Machine::rewindAndPrepareForNextLoop>,
      sc::custom_reaction<Stop> > reactions;

    sc::result react(Stop const&);
  private:
    art::IEventProcessor & ep_;
  };

  class Error : public sc::state<Error, Machine>
  {
  public:
    Error(my_context ctx);
    ~Error();
    typedef sc::transition<Stop, EndingLoop> reactions;
  private:
    art::IEventProcessor & ep_;
  };

  class HandleRuns;

  class FirstFile : public sc::state<FirstFile, HandleFiles>
  {
  public:
    FirstFile(my_context ctx);
    ~FirstFile();

    typedef mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);
  private:
    art::IEventProcessor & ep_;
  };

  class HandleNewInputFile1 : public sc::state<HandleNewInputFile1, HandleFiles>
  {
  public:
    HandleNewInputFile1(my_context ctx);
    ~HandleNewInputFile1();

    typedef mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);
  };

  class NewInputAndOutputFiles : public sc::state<NewInputAndOutputFiles, HandleFiles>
  {
  public:
    NewInputAndOutputFiles(my_context ctx);
    ~NewInputAndOutputFiles();

    typedef mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);

  private:

    void goToNewInputAndOutputFiles();

    art::IEventProcessor & ep_;
  };

  class NewRun;

  class HandleRuns : public sc::state<HandleRuns, HandleFiles, NewRun>
  {
  public:
    HandleRuns(my_context ctx);
    void exit();
    ~HandleRuns();

    typedef sc::transition<File, NewInputAndOutputFiles> reactions;

    bool beginRunCalled() const;
    art::RunID currentRun() const;
    bool runException() const;
    void setupCurrentRun();
    void beginRun(art::RunID run);
    void endRun(art::RunID run);
    void finalizeRun(Run const&);
    void finalizeRun();
    void beginRunIfNotDoneAlready();
  private:
    art::IEventProcessor & ep_;
    bool exitCalled_;
    bool beginRunCalled_;
    art::RunID currentRun_;
    std::set<art::RunID> previousRuns_;
    bool runException_;
  };

  class HandleSubRuns;

  class NewRun : public sc::state<NewRun, HandleRuns>
  {
  public:
    NewRun(my_context ctx);
    ~NewRun();

    typedef mpl::list<
      sc::transition<SubRun, HandleSubRuns>,
      sc::transition<Run, NewRun, HandleRuns, &HandleRuns::finalizeRun>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);
  };

  class ContinueRun1;

  class HandleNewInputFile2 : public sc::state<HandleNewInputFile2, HandleRuns>
  {
  public:
    HandleNewInputFile2(my_context ctx);
    ~HandleNewInputFile2();
    bool checkInvariant();

    typedef mpl::list<
      sc::custom_reaction<Run>,
      sc::custom_reaction<File> > reactions;

    sc::result react(Run const& run);
    sc::result react(File const& file);
  };

  class ContinueRun1 : public sc::state<ContinueRun1, HandleRuns>
  {
  public:
    ContinueRun1(my_context ctx);
    ~ContinueRun1();
    bool checkInvariant();

    typedef mpl::list<
      sc::transition<Run, NewRun, HandleRuns, &HandleRuns::finalizeRun>,
      sc::custom_reaction<File>,
      sc::transition<SubRun, HandleSubRuns> > reactions;

    sc::result react(File const& file);
  private:
    art::IEventProcessor & ep_;
  };

  class FirstSubRun;

  class HandleSubRuns : public sc::state<HandleSubRuns, HandleRuns, FirstSubRun>
  {
  public:
    HandleSubRuns(my_context ctx);
    void exit();
    ~HandleSubRuns();
    bool checkInvariant();

    art::SubRunID const & currentSubRun() const;
    bool currentSubRunEmpty() const;
    std::vector<art::SubRunID> const& unhandledSubRuns() const;
    void setupCurrentSubRun();
    void finalizeAllSubRuns();
    void finalizeSubRun();
    void finalizeOutstandingSubRuns();
    void markSubRunNonEmpty();

    typedef sc::transition<Run, NewRun, HandleRuns, &HandleRuns::finalizeRun> reactions;

  private:
    art::IEventProcessor & ep_;
    bool exitCalled_;
    bool currentSubRunEmpty_;
    art::SubRunID currentSubRun_;
    std::set<art::SubRunID> previousSubRuns_;
    std::vector<art::SubRunID> unhandledSubRuns_;
    bool subRunException_;
  };

  class HandleEvent;
  class AnotherSubRun;

  class FirstSubRun : public sc::state<FirstSubRun, HandleSubRuns>
  {
  public:
    FirstSubRun(my_context ctx);
    ~FirstSubRun();
    bool checkInvariant();

    typedef mpl::list<
      sc::transition<Event, HandleEvent>,
      sc::transition<SubRun, AnotherSubRun>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);
  };

  class AnotherSubRun : public sc::state<AnotherSubRun, HandleSubRuns>
  {
  public:
    AnotherSubRun(my_context ctx);
    ~AnotherSubRun();
    bool checkInvariant();

    typedef mpl::list<
      sc::transition<Event, HandleEvent>,
      sc::transition<SubRun, AnotherSubRun>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);
  };

  class HandleEvent : public sc::state<HandleEvent, HandleSubRuns>
  {
  public:
    HandleEvent(my_context ctx);
    ~HandleEvent();
    bool checkInvariant();

    typedef mpl::list<
      sc::transition<Event, HandleEvent>,
      sc::transition<SubRun, AnotherSubRun>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);
    void readAndProcessEvent();
    void markNonEmpty();
  private:
    art::IEventProcessor & ep_;
  };

  class HandleNewInputFile3 : public sc::state<HandleNewInputFile3, HandleSubRuns>
  {
  public:
    HandleNewInputFile3(my_context ctx);
    ~HandleNewInputFile3();
    bool checkInvariant();

    typedef mpl::list<
      sc::custom_reaction<Run>,
      sc::custom_reaction<File> > reactions;

    sc::result react(Run const& run);
    sc::result react(File const& file);
  };

  class ContinueRun2 : public sc::state<ContinueRun2, HandleSubRuns>
  {
  public:
    ContinueRun2(my_context ctx);
    ~ContinueRun2();
    bool checkInvariant();

    typedef mpl::list<
      sc::custom_reaction<SubRun>,
      sc::custom_reaction<File> > reactions;

    sc::result react(SubRun const& subRun);
    sc::result react(File const& file);
  private:
    art::IEventProcessor & ep_;
  };

  class ContinueSubRun : public sc::state<ContinueSubRun, HandleSubRuns>
  {
  public:
    ContinueSubRun(my_context ctx);
    ~ContinueSubRun();
    bool checkInvariant();

    typedef mpl::list<
      sc::transition<Event, HandleEvent>,
      sc::transition<SubRun, AnotherSubRun>,
      sc::custom_reaction<File> > reactions;

    sc::result react(File const& file);
  private:
    art::IEventProcessor & ep_;
  };
}

// ======================================================================

#endif /* art_Framework_EventProcessor_EPStates_h */

// Local Variables:
// mode: c++
// End:
