#include "art/Framework/Principal/SubRun.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  SubRun::~SubRun() = default;

  SubRun
  SubRun::make(SubRunPrincipal& srp,
               ModuleContext const& mc,
               RangeSet const& rs)
  {
    return SubRun{
      srp, mc, std::make_optional<ProductInserter>(InSubRun, srp, mc), rs};
  }

  SubRun
  SubRun::make(SubRunPrincipal const& srp, ModuleContext const& mc)
  {
    return SubRun{srp, mc, std::nullopt, RangeSet::invalid()};
  }

  SubRun::SubRun(SubRunPrincipal const& srp,
                 ModuleContext const& mc,
                 std::optional<ProductInserter> inserter,
                 RangeSet const& rs /* = RangeSet::invalid() */)
    : ProductRetriever{InSubRun, srp, mc, false}
    , inserter_{move(inserter)}
    , subRunPrincipal_{srp}
    , run_{Run::make(srp.runPrincipal(), mc)}
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
