#ifndef art_Framework_EventProcessor_StateMachine_Events_h
#define art_Framework_EventProcessor_StateMachine_Events_h

// ======================================================================
//
// Define the classes representing the state-machine events.
// There are seven of them.
//
// ======================================================================

#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "boost/statechart/event.hpp"

namespace sc = boost::statechart;
namespace statemachine {

  class Run : public sc::event<Run> {
  public:
    Run(art::RunID id) : id_{id} {}
    art::RunID id() const { return id_; }
  private:
    art::RunID id_;
  };

  class SubRun : public sc::event<SubRun> {
  public:
    SubRun(art::SubRunID id) : id_{id} {}
    art::SubRunID const & id() const { return id_; }
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

}

// ======================================================================

#endif /* art_Framework_EventProcessor_StateMachine_Events_h */

// Local Variables:
// mode: c++
// End:
