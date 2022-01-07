#include "art/Framework/Principal/Event.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  Event::~Event() = default;

  // FIXME: It only makes sense to track parents when putting a
  //        product onto the event.  That requires a non-const Event
  //        object.

  Event::Event(EventPrincipal const& ep,
               ModuleContext const& mc,
               bool const recordParents)
    : DataViewImpl{InEvent, ep, mc, recordParents}
    , eventPrincipal_{ep}
    , subRun_{ep.subRunPrincipalExemptPtr() ?
                new SubRun{ep.subRunPrincipal(), mc} :
                nullptr}
  {}

  EventID
  Event::id() const
  {
    return eventPrincipal_.eventID();
  }

  RunNumber_t
  Event::run() const
  {
    return id().run();
  }

  SubRunNumber_t
  Event::subRun() const
  {
    return id().subRun();
  }

  EventNumber_t
  Event::event() const
  {
    return id().event();
  }

  Timestamp
  Event::time() const
  {
    return eventPrincipal_.time();
  }

  bool
  Event::isRealData() const
  {
    return eventPrincipal_.isReal();
  }

  EventAuxiliary::ExperimentType
  Event::experimentType() const
  {
    return eventPrincipal_.ExperimentType();
  }

  History const&
  Event::history() const
  {
    return eventPrincipal_.history();
  }

  ProcessHistoryID const&
  Event::processHistoryID() const
  {
    return history().processHistoryID();
  }

  ProcessHistory const&
  Event::processHistory() const
  {
    return eventPrincipal_.processHistory();
  }

  SubRun const&
  Event::getSubRun() const
  {
    if (!subRun_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRun.\n";
    }
    return *subRun_;
  }

  Run const&
  Event::getRun() const
  {
    return getSubRun().getRun();
  }

} // namespace art
