#ifndef art_Framework_EventProcessor_StateMachine_Machine_h
#define art_Framework_EventProcessor_StateMachine_Machine_h

// ======================================================================
//
// The state machine that controls the processing of runs, subruns,
// and events is implemented using the boost statechart library and
// the states defined here.  This machine is used by the
// EventProcessor.
//
// Please see the "./doc/README" file!
//
// ======================================================================

#include "art/Framework/Core/OutputFileSwitchBoundary.h"
#include "art/Framework/EventProcessor/StateMachine/Events.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"
#include "boost/statechart/deep_history.hpp"
#include "boost/statechart/event.hpp"
#include "boost/statechart/state_machine.hpp"
#include "boost/mpl/list.hpp"
#include "boost/statechart/custom_reaction.hpp"
#include "boost/statechart/state.hpp"
#include "boost/statechart/transition.hpp"
#include "cetlib/container_algorithms.h"

#include <set>
#include <utility>
#include <vector>

namespace art {
  class EventProcessor;
}

namespace sc = boost::statechart;
namespace mpl = boost::mpl;

namespace statemachine {

  // For all these classes, the first template argument to the base
  // class is the derived class.  The second argument is the parent
  // state or if it is a top level state the Machine.  If there is a
  // third template argument it is the substate that is entered by
  // default on entry.

  class Starting;

  class Machine : public sc::state_machine<Machine, Starting> {
  public:
    Machine(art::EventProcessor* ep,
            bool handleEmptyRuns,
            bool handleEmptySubRuns);

    art::EventProcessor& ep() const;
    bool handleEmptyRuns() const;
    bool handleEmptySubRuns() const;

    void closeAllFiles();
    void closeAllFiles(Event const&);
    void closeAllFiles(SubRun const&);
    void closeAllFiles(Run const&);
    void closeAllFiles(SwitchOutputFiles const&);
    void closeAllFiles(Stop const&);

    void closeInputFile();
    void closeAllOutputFiles();
    void closeSomeOutputFiles();
    void closeSomeOutputFiles(SwitchOutputFiles const&);
    void closeInputFile(InputFile const&);

  private:

    art::EventProcessor* ep_;
    bool handleEmptyRuns_;
    bool handleEmptySubRuns_;
  };

  class Error;
  class HandleFiles;
  class Stopping;

  class Starting : public sc::state<Starting, Machine> {
  public:
    Starting(my_context ctx);

    using reactions = mpl::list<
      sc::transition<Event, Error>,
      sc::transition<SubRun, Error>,
      sc::transition<Run, Error>,
      sc::transition<InputFile, HandleFiles>,
      sc::transition<SwitchOutputFiles, Error>,
      sc::transition<Stop, Stopping> >;

  private:
    art::EventProcessor& ep_;
  };

  class NewInputFile;

  class HandleFiles : public sc::state<HandleFiles, Machine, NewInputFile, sc::has_deep_history> {
  public:
    HandleFiles(my_context ctx);

    using reactions = mpl::list<
      sc::transition<Event, Error, Machine, &Machine::closeAllFiles>,
      sc::transition<SubRun, Error, Machine, &Machine::closeAllFiles>,
      sc::transition<Run, Error, Machine, &Machine::closeAllFiles>,
      sc::transition<InputFile, HandleFiles, Machine, &Machine::closeInputFile>,
      sc::transition<SwitchOutputFiles, Error, Machine, &Machine::closeAllFiles>,
      sc::transition<Stop, Stopping, Machine, &Machine::closeAllFiles> >;

    bool stagingAllowed() const { return stagingAllowed_; }

    void disallowStaging();
    void openInputFile();
    void openSomeOutputFiles();
    void maybeTriggerOutputFileSwitch();

  private:
    art::EventProcessor& ep_;
    bool stagingAllowed_ {true};
    bool switchInProgress_ {false};
  };

  class Stopping : public sc::state<Stopping, Machine> {
  public:
    Stopping(my_context ctx);

    using reactions = sc::custom_reaction<Stop>;

    sc::result react(Stop const&);
  private:
    art::EventProcessor& ep_;
  };

  class Error : public sc::state<Error, Machine> {
  public:
    Error(my_context ctx);
    using reactions = sc::transition<Stop, Stopping>;
  private:
    art::EventProcessor& ep_;
  };

  class HandleRuns;
  class NewInputFile;

  class NewInputFile : public sc::state<NewInputFile, HandleFiles> {
  public:
    NewInputFile(my_context ctx);

    using reactions = mpl::list<
      sc::transition<Run, HandleRuns>,
      sc::custom_reaction<SwitchOutputFiles> >;

    sc::result react(SwitchOutputFiles const&);
  };

  class NewRun;

  class HandleRuns : public sc::state<HandleRuns, HandleFiles, NewRun> {
  public:
    HandleRuns(my_context ctx);
    void exit();
    ~HandleRuns();

    using reactions = sc::transition<Run, HandleRuns>;

    bool beginRunCalled() const;
    art::RunID currentRun() const;
    void setupCurrentRun();
    void beginRun();
    void endRun();
    void finalizeRun();
    void beginRunIfNotDoneAlready();

    void disableFinalizeRun(Pause const&);

  private:
    art::EventProcessor& ep_;
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

    using reactions = mpl::list<
      sc::transition<SubRun, HandleSubRuns>,
      sc::transition<Pause, PauseRun, HandleRuns, &HandleRuns::disableFinalizeRun> >;
  };

  class PauseRun : public sc::state<PauseRun, HandleRuns> {
  public:
    PauseRun(my_context ctx);

    using reactions = mpl::list<
      sc::transition<SwitchOutputFiles, sc::deep_history<HandleRuns>, Machine, &Machine::closeSomeOutputFiles>,
      sc::transition<SubRun, HandleSubRuns> >;
  };

  class NewSubRun;

  class HandleSubRuns : public sc::state<HandleSubRuns, HandleRuns, NewSubRun> {
  public:
    HandleSubRuns(my_context ctx);
    void exit();
    ~HandleSubRuns();
    void checkInvariant();

    art::SubRunID const& currentSubRun() const;
    bool beginSubRunCalled() const;

    void disableFinalizeSubRun(Pause const&);
    void beginSubRun();
    void endSubRun();

    void setupCurrentSubRun();
    void finalizeSubRun();
    void markSubRunNonEmpty();
    void beginSubRunIfNotDoneAlready();

    using reactions = sc::transition<SubRun, HandleSubRuns>;

  private:
    art::EventProcessor& ep_;
    art::SubRunID currentSubRun_;
    bool exitCalled_ {false};
    bool beginSubRunCalled_ {false};
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
      sc::transition<Pause, PauseSubRun, HandleSubRuns, &HandleSubRuns::disableFinalizeSubRun> >;
  };

  class PauseSubRun : public sc::state<PauseSubRun, HandleSubRuns> {
  public:
    PauseSubRun(my_context ctx);

    using reactions = mpl::list<
      sc::transition<SwitchOutputFiles, sc::deep_history<HandleRuns>, Machine, &Machine::closeSomeOutputFiles>,
      sc::transition<Event, HandleEvents> >;
  };

  class NewEvent;

  class HandleEvents: public sc::state<HandleEvents, HandleSubRuns, NewEvent> {
  public:
    HandleEvents(my_context ctx);
    ~HandleEvents();

    void disableFinalizeEvent(Pause const&);
    void exit();
    void finalizeEvent();
    void setCurrentEvent(art::EventID const& id) { currentEvent_ = id; }
    void setEventException(bool const value) { eventException_ = value; }
    auto const& currentEvent() const { return currentEvent_; }

    using reactions = sc::transition<Event, HandleEvents>;

  private:
    art::EventProcessor& ep_;
    art::EventID currentEvent_;
    bool exitCalled_ {false};
    bool eventException_ {false};
    bool finalizeEnabled_ {true};
  };

  class PauseEvent;
  class ProcessEvent;

  class NewEvent : public sc::state<NewEvent, HandleEvents>
  {
  public:
    NewEvent(my_context ctx);
    ~NewEvent();

    void checkInvariant();
    void markNonEmpty();

    using reactions = mpl::list<
      sc::transition<Process, ProcessEvent>,
      sc::transition<Pause, PauseEvent, HandleEvents, &HandleEvents::disableFinalizeEvent>>;

  private:
    art::EventProcessor& ep_;
  };

  class ProcessEvent: public sc::state<ProcessEvent, HandleEvents> {
  public:
    ProcessEvent(my_context ctx);
  private:
    art::EventProcessor& ep_;
  };

  class PauseEvent : public sc::state<PauseEvent, HandleEvents> {
  public:

    PauseEvent(my_context ctx);

    using reactions = mpl::list<
      sc::transition<SwitchOutputFiles, sc::deep_history<HandleRuns>, Machine, &Machine::closeSomeOutputFiles>,
      sc::transition<Process, ProcessEvent>>;
  };

}

// ======================================================================

#endif /* art_Framework_EventProcessor_StateMachine_Machine_h */

// Local Variables:
// mode: c++
// End:
