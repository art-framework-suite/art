#include "art/Framework/Principal/Run.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/RunPrincipal.h"

namespace art {

  Run::~Run() = default;

  Run::Run(RunPrincipal const& rp,
           ModuleContext const& mc,
           RangeSet const& rs /*= RangeSet::invalid()*/)
    : DataViewImpl{InRun, rp, mc, false}, runPrincipal_{rp}, rangeSet_{rs}
  {}

  RunID
  Run::id() const
  {
    return runPrincipal_.runID();
  }

  RunNumber_t
  Run::run() const
  {
    return id().run();
  }

  Timestamp const&
  Run::beginTime() const
  {
    return runPrincipal_.beginTime();
  }

  Timestamp const&
  Run::endTime() const
  {
    return runPrincipal_.endTime();
  }

  ProcessHistory const&
  Run::processHistory() const
  {
    return runPrincipal_.processHistory();
  }

} // namespace art
