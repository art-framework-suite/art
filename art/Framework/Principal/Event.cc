#include "art/Framework/Principal/Event.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  Event::~Event() = default;

  Event::Event(EventPrincipal const& ep,
               ModuleContext const& mc,
               std::optional<ProductInserter> inserter)
    : ProductRetriever{InEvent, ep, mc, inserter.has_value()}
    , inserter_{move(inserter)}
    , eventPrincipal_{ep}
    , subRun_{ep.subRunPrincipal().makeSubRun(mc)}
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

  ProcessHistoryID const&
  Event::processHistoryID() const
  {
    return eventPrincipal_.processHistoryID();
  }

  ProcessHistory const&
  Event::processHistory() const
  {
    return eventPrincipal_.processHistory();
  }

  SubRun const&
  Event::getSubRun() const
  {
    return subRun_;
  }

  Run const&
  Event::getRun() const
  {
    return getSubRun().getRun();
  }

  void
  Event::commitProducts()
  {
    assert(inserter_);
    inserter_->commitProducts();
  }

  void
  Event::commitProducts(
    bool const checkProducts,
    std::map<TypeLabel, BranchDescription> const* expectedProducts)
  {
    assert(inserter_);
    inserter_->commitProducts(
      checkProducts, expectedProducts, ProductRetriever::retrievedPIDs());
  }

} // namespace art
