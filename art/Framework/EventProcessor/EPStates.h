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

  enum FileMode { NOMERGE, MERGE };

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

  // It is slightly confusing that this one refers to both physics
  // event and a boost statechart event ...
  class Event : public sc::event<Event> { };

  class InputFile : public sc::event<InputFile> {};
  class Stop : public sc::event<Stop> {};

  // Now define the machine and the states.  For all these classes,
  // the first template argument to the base class is the derived
  // class.  The second argument is the parent state or if it is a top
  // level state the Machine.  If there is a third template argument
  // it is the substate that is entered by default on entry.

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

  private:

    art::IEventProcessor* ep_;
    FileMode fileMode_;
    bool handleEmptyRuns_;
    bool handleEmptySubRuns_;
  };

  class Error;
  class HandleFiles;
  class Stopping;

  class Starting : public sc::state<Starting, Machine>
  {
  public:
    Starting(my_context ctx);
    ~Starting();

    using reactions = mpl::list<
      sc::transition<Event, Error>,
      sc::transition<SubRun, Error>,
      sc::transition<Run, Error>,
      sc::transition<InputFile, HandleFiles>,
      sc::transition<Stop, Stopping> >;
  };

  class FirstFile;

  class HandleFiles : public sc::state<HandleFiles, Machine, FirstFile>
  {
  public:
    HandleFiles(my_context ctx);
    void exit();
    ~HandleFiles();

    using reactions = mpl::list<
      sc::transition<Event, Error>,
      sc::transition<SubRun, Error>,
      sc::transition<Run, Error>,
      sc::transition<InputFile, Error>,
      sc::transition<Stop, Stopping> >;

    void openFiles();
    void closeFiles();
    void goToNewInputFile();
    void goToNewInputAndOutputFiles();
    void goToNewOutputFiles();
    bool shouldWeCloseOutput();
  private:
    art::IEventProcessor & ep_;
    bool exitCalled_;
  };

  class Stopping : public sc::state<Stopping, Machine>
  {
  public:
    Stopping(my_context ctx);
    ~Stopping() = default;

    using reactions = sc::custom_reaction<Stop>;

    sc::result react(Stop const&);
  private:
    art::IEventProcessor & ep_;
  };

  class Error : public sc::state<Error, Machine>
  {
  public:
    Error(my_context ctx);
    ~Error() = default;
    using reactions = sc::transition<Stop, Stopping>;
  private:
    art::IEventProcessor & ep_;
  };

  class HandleRuns;

  class FirstFile : public sc::state<FirstFile, HandleFiles>
  {
  public:
    FirstFile(my_context ctx);
    ~FirstFile() = default;

    using reactions = mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::custom_reaction<InputFile> >;

    sc::result react(InputFile const& file);
  };

  class NewInputFile : public sc::state<NewInputFile, HandleFiles>
  {
  public:
    NewInputFile(my_context ctx);
    ~NewInputFile() = default;

    using reactions = mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::custom_reaction<InputFile> >;

    sc::result react(InputFile const& file);
  };

  class NewInputAndOutputFiles : public sc::state<NewInputAndOutputFiles, HandleFiles>
  {
  public:
    NewInputAndOutputFiles(my_context ctx);
    ~NewInputAndOutputFiles() = default;

    using reactions = mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::custom_reaction<InputFile> >;

    sc::result react(InputFile const& file);
  };

  class NewRun;

  class HandleRuns : public sc::state<HandleRuns, HandleFiles, NewRun>
  {
  public:
    HandleRuns(my_context ctx);
    void exit();
    ~HandleRuns();

    using reactions = sc::transition<InputFile, NewInputAndOutputFiles>;

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

    using reactions = mpl::list<
      sc::transition<SubRun, HandleSubRuns>,
      sc::transition<Run, NewRun, HandleRuns, &HandleRuns::finalizeRun>,
      sc::custom_reaction<InputFile> >;

    sc::result react(InputFile const& file);
  };

  class NewSubRun;

  class HandleSubRuns : public sc::state<HandleSubRuns, HandleRuns, NewSubRun>
  {
  public:
    HandleSubRuns(my_context ctx);
    void exit();
    ~HandleSubRuns();
    void checkInvariant();

    art::SubRunID const & currentSubRun() const;
    bool currentSubRunEmpty() const;
    std::vector<art::SubRunID> const& unhandledSubRuns() const;
    void setupCurrentSubRun();
    void finalizeAllSubRuns();
    void finalizeSubRun(SubRun const&);
    void finalizeSubRun();
    void finalizeOutstandingSubRuns();
    void markSubRunNonEmpty();

    using reactions = sc::transition<Run, NewRun, HandleRuns, &HandleRuns::finalizeRun>;

  private:
    art::IEventProcessor & ep_;
    bool exitCalled_;
    bool currentSubRunEmpty_;
    art::SubRunID currentSubRun_;
    std::set<art::SubRunID> previousSubRuns_;
    std::vector<art::SubRunID> unhandledSubRuns_;
    bool subRunException_;
  };

  class NewEvent;

  class NewSubRun : public sc::state<NewSubRun, HandleSubRuns>
  {
  public:
    NewSubRun(my_context ctx);
    ~NewSubRun();
    void checkInvariant();

    using reactions = mpl::list<
      sc::transition<Event, NewEvent>,
      sc::transition<SubRun, NewSubRun, HandleSubRuns, &HandleSubRuns::finalizeSubRun>,
      sc::custom_reaction<InputFile> >;

    sc::result react(InputFile const& file);
  };

  class NewEvent : public sc::state<NewEvent, HandleSubRuns>
  {
  public:
    NewEvent(my_context ctx);
    ~NewEvent();
    void checkInvariant();

    using reactions = mpl::list<
      sc::transition<Event, NewEvent>,
      sc::transition<SubRun, NewSubRun, HandleSubRuns, &HandleSubRuns::finalizeSubRun>,
      sc::custom_reaction<InputFile> >;

    sc::result react(InputFile const& file);
    void readAndProcessEvent();
    void markNonEmpty();
  private:
    art::IEventProcessor & ep_;
  };

}

// ======================================================================

#endif /* art_Framework_EventProcessor_EPStates_h */

// Local Variables:
// mode: c++
// End:
