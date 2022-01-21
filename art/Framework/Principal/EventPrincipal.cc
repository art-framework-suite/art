#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Event.h"

// vim: set sw=2 expandtab :

namespace art {

  EventPrincipal::~EventPrincipal() = default;

  EventPrincipal::EventPrincipal(
    EventAuxiliary const& aux,
    ProcessConfiguration const& pc,
    cet::exempt_ptr<ProductTable const> presentProducts,
    std::unique_ptr<DelayedReader>&&
      reader /*= std::make_unique<NoDelayedReader>()*/,
    bool const lastInSubRun /*= false*/)
    : Principal{InEvent,
                pc,
                presentProducts,
                aux.processHistoryID(),
                move(reader)}
    , aux_{aux}
    , lastInSubRun_{lastInSubRun}
  {}

  Event
  EventPrincipal::makeEvent(ModuleContext const& mc)
  {
    return Event{*this, mc, makeInserter(mc)};
  }

  Event
  EventPrincipal::makeEvent(ModuleContext const& mc) const
  {
    return Event{*this, mc};
  }

  EventAuxiliary const&
  EventPrincipal::eventAux() const
  {
    return aux_;
  }

  EventID const&
  EventPrincipal::eventID() const
  {
    return aux_.id();
  }

  EventNumber_t
  EventPrincipal::event() const
  {
    return aux_.id().event();
  }

  SubRunNumber_t
  EventPrincipal::subRun() const
  {
    return aux_.id().subRun();
  }

  RunNumber_t
  EventPrincipal::run() const
  {
    return aux_.id().run();
  }

  Timestamp const&
  EventPrincipal::time() const
  {
    return aux_.time();
  }

  bool
  EventPrincipal::isReal() const
  {
    return aux_.isRealData();
  }

  EventAuxiliary::ExperimentType
  EventPrincipal::ExperimentType() const
  {
    return aux_.experimentType();
  }

  bool
  EventPrincipal::isLastInSubRun() const
  {
    return lastInSubRun_;
  }

  SubRunPrincipal const&
  EventPrincipal::subRunPrincipal() const
  {
    if (subRunPrincipal_.get() == nullptr) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
    }
    return *subRunPrincipal_;
  }

  void
  EventPrincipal::setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal const> srp)
  {
    subRunPrincipal_ = srp;
  }

  void
  EventPrincipal::refreshProcessHistoryID()
  {
    aux_.setProcessHistoryID(processHistoryID());
  }

  void
  EventPrincipal::createGroupsForProducedProducts(
    ProductTables const& producedProducts)
  {
    Principal::createGroupsForProducedProducts(producedProducts);
    refreshProcessHistoryID();
  }
} // namespace art
