#include "art/Framework/Principal/SubRun.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  SubRun::~SubRun() = default;

  SubRun::SubRun(SubRunPrincipal const& srp,
                 ModuleContext const& mc,
                 std::optional<ProductInserter> inserter,
                 RangeSet const& rs /* = RangeSet::invalid() */)
    : ProductRetriever{InSubRun, srp, mc, false}
    , inserter_{std::move(inserter)}
    , subRunPrincipal_{srp}
    , run_{srp.runPrincipal().makeRun(mc)}
    , rangeSet_{rs}
  {}

  SubRunID
  SubRun::id() const
  {
    return subRunPrincipal_.subRunID();
  }

  RunNumber_t
  SubRun::run() const
  {
    return id().run();
  }

  RunNumber_t
  SubRun::subRun() const
  {
    return id().subRun();
  }

  Timestamp const&
  SubRun::beginTime() const
  {
    return subRunPrincipal_.beginTime();
  }

  Timestamp const&
  SubRun::endTime() const
  {
    return subRunPrincipal_.endTime();
  }

  ProcessHistory const&
  SubRun::processHistory() const
  {
    return subRunPrincipal_.processHistory();
  }

  Run const&
  SubRun::getRun() const
  {
    return run_;
  }

  void
  SubRun::commitProducts()
  {
    assert(inserter_);
    inserter_->commitProducts();
  }

} // namespace art
