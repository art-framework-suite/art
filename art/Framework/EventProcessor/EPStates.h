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

#include "art/Framework/Core/IEventProcessor.h"
#include "art/Framework/Core/OutputFileSwitchBoundary.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "art/Utilities/Exception.h"
#include "boost/statechart/deep_history.hpp"
#include "boost/statechart/event.hpp"
#include "boost/statechart/state_machine.hpp"
#include "boost/mpl/list.hpp"
#include "boost/statechart/custom_reaction.hpp"
#include "boost/statechart/state.hpp"
#include "boost/statechart/transition.hpp"
#include "cetlib/container_algorithms.h"

#include <iostream>
#include <set>
#include <utility>
#include <vector>

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

namespace statemachine {

  // Define the classes representing the "boost statechart events".
  // There are seven of them.

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
  class SwitchOutputFiles : public sc::event<SwitchOutputFiles> {};

  class Stop : public sc::event<Stop> {};
  class Pause : public sc::event<Pause> {};

  // Now define the machine and the states.  For all these classes,
  // the first template argument to the base class is the derived
  // class.  The second argument is the parent state or if it is a top
  // level state the Machine.  If there is a third template argument
  // it is the substate that is entered by default on entry.

  class Starting;

  class Machine : public sc::state_machine<Machine, Starting> {
  public:
    Machine(art::IEventProcessor* ep,
            bool handleEmptyRuns,
            bool handleEmptySubRuns);

    art::IEventProcessor& ep() const;
    bool handleEmptyRuns() const;
    bool handleEmptySubRuns() const;

  private:

    art::IEventProcessor* ep_;
    bool handleEmptyRuns_;
    bool handleEmptySubRuns_;
  };

  class Error;
  class HandleFiles;
  class Stopping;

  class Starting : public sc::state<Starting, Machine> {
  public:
    Starting(my_context ctx);
    ~Starting();

    using reactions = mpl::list<
      sc::transition<Event, Error>,
      sc::transition<SubRun, Error>,
      sc::transition<Run, Error>,
      sc::transition<InputFile, HandleFiles>,
      sc::transition<SwitchOutputFiles, Error>,
      sc::transition<Stop, Stopping> >;

  private:
    art::IEventProcessor& ep_;
  };

  class FirstFile;

  class HandleFiles : public sc::state<HandleFiles, Machine, FirstFile, sc::has_deep_history> {
  public:
    HandleFiles(my_context ctx);
    void exit();
    ~HandleFiles();

    using reactions = mpl::list<
      sc::transition<Event, Error>,
      sc::transition<SubRun, Error>,
      sc::transition<Run, Error>,
      sc::transition<InputFile, Error>,
      sc::transition<SwitchOutputFiles, Error>,
      sc::transition<Stop, Stopping> >;

    void openAllFiles();
    void closeAllFiles();
    void goToNewInputFile();

    void maybeOpenOutputFiles();
    void maybeCloseOutputFiles();
    void switchOutputFiles(SwitchOutputFiles const&);

    void setCurrentBoundary(art::Boundary::BT const);
    void maybeTriggerOutputFileSwitch(art::Boundary::BT const);

  private:
    art::IEventProcessor & ep_;
    bool exitCalled_ {false};
    art::Boundary::BT currentBoundary_ {art::Boundary::Unset};
    art::Boundary::BT previousBoundary_ {art::Boundary::Unset};
    bool switchInProgress_ {false};

  };

  class Stopping : public sc::state<Stopping, Machine> {
  public:
    Stopping(my_context ctx);
    ~Stopping() = default;

    using reactions = sc::custom_reaction<Stop>;

    sc::result react(Stop const&);
  private:
    art::IEventProcessor & ep_;
  };

  class Error : public sc::state<Error, Machine> {
  public:
    Error(my_context ctx);
    ~Error() = default;
    using reactions = sc::transition<Stop, Stopping>;
  private:
    art::IEventProcessor & ep_;
  };

  class HandleRuns;
  class NewInputFile;

  class FirstFile : public sc::state<FirstFile, HandleFiles> {
  public:
    FirstFile(my_context ctx);
    ~FirstFile() = default;

    using reactions = mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::transition<InputFile, NewInputFile> >;
  };

  class NewInputFile : public sc::state<NewInputFile, HandleFiles> {
  public:
    NewInputFile(my_context ctx);
    ~NewInputFile();

    using reactions = mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::transition<InputFile, NewInputFile>,
      sc::custom_reaction<SwitchOutputFiles> >;

    sc::result react(SwitchOutputFiles const&);
  };

  class NewRun;

  class HandleRuns : public sc::state<HandleRuns, HandleFiles, NewRun> {
  public:
    HandleRuns(my_context ctx);
    void exit();
    ~HandleRuns();

    using reactions = mpl::list<
      sc::transition<InputFile, NewInputFile>,
      sc::transition<Run, HandleRuns> >;

    bool beginRunCalled() const;
    art::RunID currentRun() const;
    bool runException() const;
    void setupCurrentRun();
    void beginRun(art::RunID run);
    void endRun(art::RunID run);
    void finalizeRun(Run const&);
    void finalizeRun();
    void beginRunIfNotDoneAlready();

    void disableFinalizeRun(Pause const&) { finalizeEnabled_ = false; }
    void resumeAndFinalizeRun(Run const&);
    void resume(SubRun const&);

  private:

    void resetFormerState();

    art::IEventProcessor & ep_;
    art::RunID currentRun_;
    bool exitCalled_ {false};
    bool beginRunCalled_ {false};
    bool runException_ {false};
    bool finalizeEnabled_ {true};

  };

  class HandleSubRuns;
  class PauseRun;

  class NewRun : public sc::state<NewRun, HandleRuns> {
  public:
    NewRun(my_context ctx);
    ~NewRun();

    using reactions = mpl::list<
      sc::transition<SubRun, HandleSubRuns>,
      sc::transition<Pause, PauseRun, HandleRuns, &HandleRuns::disableFinalizeRun> >;
  };

  class PauseRun : public sc::state<PauseRun, HandleRuns> {
  public:
    PauseRun(my_context ctx);
    ~PauseRun();

    using reactions = mpl::list<
      sc::transition<SwitchOutputFiles,sc::deep_history<HandleRuns>, HandleFiles, &HandleFiles::switchOutputFiles>,
      sc::transition<Run, NewRun, HandleRuns, &HandleRuns::resumeAndFinalizeRun>,
      sc::transition<SubRun, HandleSubRuns, HandleRuns, &HandleRuns::resume> >;
  };

  class NewSubRun;

  class HandleSubRuns : public sc::state<HandleSubRuns, HandleRuns, NewSubRun> {
  public:
    HandleSubRuns(my_context ctx);
    void exit();
    ~HandleSubRuns();
    void checkInvariant();

    art::SubRunID const & currentSubRun() const;
    bool currentSubRunEmpty() const;
    std::vector<art::SubRunID> const& unhandledSubRuns() const;

    void disableFinalizeSubRun(Pause const&) { finalizeEnabled_ = false; }
    void resumeAndFinalizeSubRun(SubRun const&);
    void resume(Event const&);

    void setupCurrentSubRun();
    void finalizeAllSubRuns();
    void finalizeSubRun(SubRun const&);
    void finalizeSubRun();
    void finalizeOutstandingSubRuns();
    void markSubRunNonEmpty();

    using reactions = mpl::list< sc::transition<SubRun,HandleSubRuns> >;

  private:
    void resetFormerState();

    art::IEventProcessor & ep_;
    art::SubRunID currentSubRun_;
    std::vector<art::SubRunID> unhandledSubRuns_;
    bool exitCalled_ {false};
    bool currentSubRunEmpty_ {false};
    bool subRunException_ {false};
    bool finalizeEnabled_ {true};
  };

  class HandleEvents;
  class PauseSubRun;

  class NewSubRun : public sc::state<NewSubRun, HandleSubRuns> {
  public:
    NewSubRun(my_context ctx);
    ~NewSubRun();
    void checkInvariant();

    using reactions = mpl::list<
      sc::transition<Event, HandleEvents>,
      sc::transition<SubRun, NewSubRun, HandleSubRuns, &HandleSubRuns::finalizeSubRun>,
      sc::transition<Pause, PauseSubRun, HandleSubRuns, &HandleSubRuns::disableFinalizeSubRun> >;
  };

  class PauseSubRun : public sc::state<PauseSubRun, HandleSubRuns> {
  public:
    PauseSubRun(my_context ctx);
    ~PauseSubRun();

    using reactions = mpl::list<
      sc::transition<SwitchOutputFiles,sc::deep_history<HandleRuns>, HandleFiles, &HandleFiles::switchOutputFiles>,
      sc::transition<SubRun, NewSubRun, HandleSubRuns, &HandleSubRuns::resumeAndFinalizeSubRun>,
      sc::transition<Event, HandleEvents, HandleSubRuns, &HandleSubRuns::resume> >;
  };

  class NewEvent;

  class HandleEvents: public sc::state<HandleEvents, HandleSubRuns, NewEvent> {
  public:
    HandleEvents(my_context ctx);
    ~HandleEvents();

    void disableFinalizeEvent(Pause const&) { finalizeEnabled_ = false; }
    void exit();
    void finalizeEvent();
    void checkInvariant();
    void resumeAndFinalizeEvent(Event const&);

    using reactions = sc::transition<SubRun, NewSubRun, HandleSubRuns, &HandleSubRuns::finalizeSubRun>;

  private:
    art::IEventProcessor & ep_;
    art::EventID currentEvent_;
    bool exitCalled_ {false};
    bool finalizeEnabled_ {true};
  };

  class PauseEvent;

  class NewEvent : public sc::state<NewEvent, HandleEvents>
  {
  public:
    NewEvent(my_context ctx);
    ~NewEvent();

    void checkInvariant();
    void markNonEmpty();
    void readAndProcessEvent();

    using reactions = mpl::list<
      sc::transition<Event, HandleEvents>,
      sc::transition<Pause, PauseEvent, HandleEvents, &HandleEvents::disableFinalizeEvent> >;

  private:
    art::IEventProcessor & ep_;
  };

  class PauseEvent : public sc::state<PauseEvent, HandleEvents> {
  public:
    PauseEvent(my_context ctx);
    ~PauseEvent();

    using reactions = mpl::list<
      sc::transition<SwitchOutputFiles,sc::deep_history<HandleRuns>, HandleFiles, &HandleFiles::switchOutputFiles>,
      sc::transition<Event, NewEvent, HandleEvents, &HandleEvents::resumeAndFinalizeEvent> >;
  };

}

// ======================================================================

#endif /* art_Framework_EventProcessor_EPStates_h */

// Local Variables:
// mode: c++
// End:
