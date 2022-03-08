#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Run.h"
// vim: set sw=2 expandtab :

namespace art {

  RunPrincipal::~RunPrincipal() = default;

  RunPrincipal::RunPrincipal(
    RunAuxiliary const& aux,
    ProcessConfiguration const& pc,
    cet::exempt_ptr<ProductTable const> presentProducts,
    std::unique_ptr<DelayedReader>&&
      reader /*= std::make_unique<NoDelayedReader>()*/)
    : Principal{InRun,
                pc,
                presentProducts,
                aux.processHistoryID(),
                move(reader)}
    , aux_{aux}
  {}

  Run
  RunPrincipal::makeRun(ModuleContext const& mc, RangeSet const& rs)
  {
    return Run{*this, mc, makeInserter(mc), rs};
  }

  Run
  RunPrincipal::makeRun(ModuleContext const& mc) const
  {
    return Run{*this, mc};
  }

  Timestamp const&
  RunPrincipal::beginTime() const
  {
    return aux_.beginTime();
  }

  Timestamp const&
  RunPrincipal::endTime() const
  {
    return aux_.endTime();
  }

  RunAuxiliary const&
  RunPrincipal::runAux() const
  {
    return aux_;
  }

  RunID
  RunPrincipal::runID() const
  {
    return aux_.id();
  }

  RunNumber_t
  RunPrincipal::run() const
  {
    return aux_.run();
  }

  void
  RunPrincipal::createGroupsForProducedProducts(
    ProductTables const& producedProducts)
  {
    Principal::createGroupsForProducedProducts(producedProducts);
    aux_.setProcessHistoryID(processHistoryID());
  }
} // namespace art
