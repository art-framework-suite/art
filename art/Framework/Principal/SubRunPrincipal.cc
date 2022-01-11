#include "art/Framework/Principal/SubRunPrincipal.h"
// vim: set sw=2 expandtab :

namespace art {

  SubRunPrincipal::~SubRunPrincipal() = default;

  SubRunPrincipal::SubRunPrincipal(
    SubRunAuxiliary const& aux,
    ProcessConfiguration const& pc,
    cet::exempt_ptr<ProductTable const> presentProducts,
    std::unique_ptr<DelayedReader>&&
      reader /*= std::make_unique<NoDelayedReader>()*/)
    : Principal{InSubRun,
                pc,
                presentProducts,
                aux.processHistoryID(),
                move(reader)}
    , aux_{aux}
  {}

  SubRunAuxiliary const&
  SubRunPrincipal::subRunAux() const
  {
    return aux_;
  }

  SubRunID
  SubRunPrincipal::subRunID() const
  {
    return aux_.id();
  }

  SubRunNumber_t
  SubRunPrincipal::subRun() const
  {
    return aux_.subRun();
  }

  Timestamp const&
  SubRunPrincipal::beginTime() const
  {
    return aux_.beginTime();
  }

  Timestamp const&
  SubRunPrincipal::endTime() const
  {
    return aux_.endTime();
  }

  RunPrincipal const&
  SubRunPrincipal::runPrincipal() const
  {
    if (!runPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL runPrincipal.\n";
    }
    return *runPrincipal_;
  }

  RunID const&
  SubRunPrincipal::runID() const
  {
    return aux_.runID();
  }

  SubRunNumber_t
  SubRunPrincipal::run() const
  {
    return aux_.run();
  }

  void
  SubRunPrincipal::setRunPrincipal(cet::exempt_ptr<RunPrincipal const> rp)
  {
    runPrincipal_ = rp;
  }

  void
  SubRunPrincipal::createGroupsForProducedProducts(
    ProductTables const& producedProducts)
  {
    Principal::createGroupsForProducedProducts(producedProducts);
    aux_.setProcessHistoryID(processHistoryID());
  }

} // namespace art
