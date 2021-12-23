#include "art/Framework/Principal/Run.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/RunPrincipal.h"

namespace art {

  Run::~Run() = default;

  Run::Run(RunPrincipal const& rp,
           ModuleContext const& mc,
           RangeSet const& rs /*= RangeSet::invalid()*/)
    : DataViewImpl{InRun, rp, mc, false}
    , rangeSet_{rs}
  {}

  RunID
  Run::id() const
  {
    return DataViewImpl::runID();
  }

} // namespace art
