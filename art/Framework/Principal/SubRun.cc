#include "art/Framework/Principal/SubRun.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  SubRun::~SubRun() = default;

  SubRun::SubRun(SubRunPrincipal const& srp,
                 ModuleContext const& mc,
                 RangeSet const& rs /* = RangeSet::invalid() */)
    : DataViewImpl{InSubRun, srp, mc, false}
    , run_{srp.runPrincipalExemptPtr() ? new Run{srp.runPrincipal(), mc} :
                                         nullptr}
    , rangeSet_{rs}
  {}

  SubRunID
  SubRun::id() const
  {
    return DataViewImpl::subRunID();
  }

  Run const&
  SubRun::getRun() const
  {
    if (!run_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL run.\n";
    }
    return *run_;
  }

} // namespace art
